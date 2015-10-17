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
#include "adc.h"
#include "pwm.h"


Result_Buffer res_buf;

ISR(ADC_vect)
{
	printf ("result: %d\r", ADC);
	adc_start(OPAMP_CHAN);
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
	
	OCR1A = 1000;
	ICR1 =  30000;
}

int main(void) 
{
	DDRB |= 3;
	PORTB |= 1;


	uart_init();
	pwm_init();
	//adc_init();
	sei();

	printf("Levitron collider started...\n\r");

	while (1) {
		printf("do work...\n\r");
		_delay_ms(500);		
		TGLBIT(PORTB,0);
	}
	return 0;
}
