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


void timer_init()
{
	TCCR0 =(1 << CS01) | (1 << CS00);
	TIMSK |= 1 << TOIE0;
}

void adc_init()
{
	ADMUX = (1<<REFS1) | (1<<REFS0) | OPAMP_CHAN; 
	ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (1<<ADPS0);
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
	OCR1A = 200;
	ICR1 =  1000;
}

// interrupt on compare PWM timer
/*ISR(TIMER1_COMPA_vect)
{
	if (TCNT1 > 100) {
	SETBIT(PORTB,0);
	_delay_us(10);
	CLRBIT(PORTB,0);
	}
}*/


Result_Buffer sensor_buf[2];
int adjust = 0;

void add_result(int ind, int result)
{
	sensor_buf[ind].buf[sensor_buf[ind].curr=(++sensor_buf[ind].curr)%_BUF_SIZE] = result;
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
	int cur = value*9;
	OCR1A = cur;
}


int new = 0;
int diff = 0;
int Kp = 12;
int Kd = 10;
int S = 0;
int Op = 0;
int Od = 0;

int abs (int val)
{
	return (val>0) ? val : -val;
}

void do_levitate()
{
	SETBIT(PORTB,0);
	int up = filter_signal(0);
	int dn = filter_signal(1);
	diff = dn-up-100;
	
	Kd = 20*adjust/1000;

	static int cnt = 0;
	static int prevDiff = 0;
	if (++cnt > 10) {
		cnt = 0;
		S = diff - prevDiff;
		prevDiff = diff;
	}

	if (diff < -50)
		return;

	Op = (Kp*diff)/10; // пропорциональная составляющая
	Od = (Kd*S)/10;    // дифференциальная составляющая
	new = 30 - Op - Od;
	
	if (new < 0)
		new = 0;
	if (new > 50)
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


	uart_init();
	printf("Levitron collider started...\n\r");
	pwm_init();
	adc_init();
	timer_init();
	sei();

	
	while (1) {
		up = filter_signal(0);
		dn = filter_signal(1);
		printf("up:%d dn:%d diff:%d\t"
				    "Kp:%d, Kd:%d diff:%d\tOp:%d\tOd:%d\tnew=%d\n\r", 
						up, dn, dn-up, 
						Kp, Kd, diff, Op, Od , new);
		_delay_ms(30);		
		//TGLBIT(PORTB,0);
	}
	return 0;
}

ISR(TIMER0_OVF_vect)
{
	do_levitate();
}
