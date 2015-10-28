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
	TCCR0 = (1 << CS02);
	TIMSK |= 1 << TOIE0;
}

void adc_init()
{
	ADMUX = (1<<REFS1) | (1<<REFS0) | OPAMP_CHAN; 
	ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
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
	TCCR1B = (1<< WGM13) | (1<<CS10);   // clk/1
	
	OCR1A = 6000;
	ICR1 =  10000;
}


Result_Buffer sensor_buf[2];

void add_result(int ind, int result)
{
	sensor_buf[ind].buf[sensor_buf[ind].curr=(++sensor_buf[ind].curr)%_BUF_SIZE] = result;
}

ISR(ADC_vect)
{
	static int channel;

	if (channel == S_UP) {
		add_result(0, ADC);
		channel = S_DOWN;
	} else {
		add_result(1, ADC);
		channel = S_UP;
	}
	adc_start(channel);
	TGLBIT(PORTB,0);
}

ISR(TIMER0_OVF_vect)
{
	do_levitate();
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
	int cur = value*90;
	OCR1A = cur;
}


#define K 1
int new = 0;
int diff = 0;
void do_levitate()
{
	int up = filter_signal(0);
	int dn = filter_signal(1);
	diff = dn-up-60;
	new = 60 - (K*diff);
	change_pwm(new);
	
}

int main(void) 
{
	int up, dn = 0;

	DDRB |= 3;
	PORTB |= 1;


	uart_init();
	pwm_init();
	adc_init();
	timer_init();
	sei();

	printf("Levitron collider started...\n\r");
	
	while (1) {
		up = filter_signal(0);
		dn = filter_signal(1);
		printf("adc:up:%d dn:%d diff:%d\t diff=%d, new=%d\n\r", up, dn, dn-up, diff, new);
		_delay_ms(300);		
		TGLBIT(PORTB,0);
	}
	return 0;
}
