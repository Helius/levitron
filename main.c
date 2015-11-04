#include <avr/io.h>
//#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//#include <avr/eeprom.h>
//#include <avr/sleep.h>
//#include <avr/pgmspace.h>
//#include <string.h>

#include "main.h"
#include "uart.h"

#define MODE_CALIBRATE 0
#define MODE_LEVITATE  1

char mode;
int calibration[2][20];// store sensor error signal for different PWM value

void timer_init()
{
	TCCR0 = (0 << CS02) | (1 << CS01) | (0 << CS00);
	TIMSK |= 1 << TOIE0;
}

void adc_init()
{
	ADMUX = (1<<REFS1) | (1<<REFS0) | OPAMP_CHAN; 
	ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
	ADCSRA |= (1<<ADSC);
}

void adc_start(int channel)
{
	ADMUX &= 0xF0;
	ADMUX |= channel & 0x0F;
	ADCSRA |= (1<<ADSC);
}

void pwm_init()
{
	TCCR1A = (1<<COM1A1) | (1<<WGM12) | (1<<WGM11) | (0<<WGM10); // fast pwm with ICR1 top
	TCCR1B = (1 << WGM13) | (1<<CS10);   // clk/1
	//TIMSK |= (1 << OCIE1A); // interrupt on compare
	// init value of pwm	
	OCR1A = 0; // should be 0 for calibration
	ICR1 =  400;
}


Result_Buffer sensor_buf[2];
int adjust = 0;

void add_result(int ind, int result)
{
	sensor_buf[ind].buf[sensor_buf[ind].curr = (++sensor_buf[ind].curr)%_BUF_SIZE] 
		= result;
}

ISR(ADC_vect)
{
	//SETBIT(PORTB,0);
	static int channel;

	if (channel == S_UP) {
		add_result(0, ADC);
		channel = S_DOWN;
	} else if (channel == S_DOWN){
		add_result(1, ADC);
		channel = CALIB_CHAN;
	} else {
		adjust = ADC;
		channel = S_UP;
	}
	adc_start(channel);
	//CLRBIT(PORTB,0);
}

int filter_signal(int ind)
{
	int sum = 0;
	for (int i = 0; i < _BUF_SIZE; i++)
		sum += sensor_buf[ind].buf[i];
	return sum/_BUF_SIZE;
}

void change_pwm(int value)
{
	OCR1A = (ICR1*value)/111;
}

void print_calibrate()
{
	printf("- calibrate table: -\n\r");
	for (int i = 0; i < 20; i++)
		printf("%d\t%d\n\r", calibration[0][i], calibration[1][i]);
	printf("-----------\n\r");
	
}

int new = 0;
int diff = 0;
int S = 0;
int Op = 0;
int Od = 0;
int calibrate_index = 0;

int Kp = 12;
int Kd = 28;
unsigned int limit = 25;


void do_levitate()
{
	SETBIT(PORTB,0);
	static int cnt = 0;
	int up = filter_signal(0);
	int dn = filter_signal(1);
	int true_diff = dn-up;

	if (mode == MODE_CALIBRATE) {
		if (++cnt > 100) {
			calibration[0][calibrate_index] = OCR1A;
			calibration[1][calibrate_index] = true_diff;
			if(++calibrate_index == 20) { // we've done
				print_calibrate();
				mode = MODE_LEVITATE;
				OCR1A = ICR1/4;
				return;
			}
			OCR1A = calibrate_index * (ICR1/40);
			cnt = 0;
		} 
		return;
	}

	calibrate_index = OCR1A*40/ICR1;
	if (calibrate_index >= 20)
		calibrate_index = 19;
	diff = dn - up - 100 - calibration[1][calibrate_index];

	static int prevDiff = 0;
	if (++cnt > limit) {
		cnt = 0;
		S = (diff - prevDiff);
		prevDiff = diff;
	}

	if (diff < -70) {
		change_pwm(0);
		return;
	}

	Op = (Kp*diff)/10; // пропорциональная составляющая
	Od = (Kd*S)/10;    // дифференциальная составляющая
	new = new/2 + (40 - Op - Od)/2;

	if (new < 0)
		new = 0;
	if (new > 60)
		return;
	change_pwm(new);
	CLRBIT(PORTB,0);

}

int main(void) 
{
	int up, dn = 0;

	DDRB |= 3;
	PORTB |= 1;

	SETBIT(PORTB,0);
	
	mode = MODE_CALIBRATE;

	uart_init();
	printf("Levitron collider started...\n\r");
	pwm_init();
	adc_init();
	timer_init();
	sei();

	
	while (1) {
		if (1 || mode == MODE_LEVITATE) {
			up = filter_signal(0);
			dn = filter_signal(1);
			printf("up:%d dn:%d diff:%d\t"
					"Kp:%d, Kd:%d[%d] diff:%d\tOp:%d\tOd:%d\tnew=%d[%d]{%d}\n\r", 
					up, dn, dn-up, 
					Kp, Kd, limit, diff, Op, Od , new, OCR1A, calibration[1][calibrate_index]);
			_delay_ms(30);		
			//TGLBIT(PORTB,0);
		}
	}
	return 0;
}

ISR(TIMER0_OVF_vect)
{
	static int i = 0;
	if ((++i)&1)
		do_levitate();
}
