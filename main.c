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


Result_Buffer sensor_buf;

void add_result(int result)
{
	sensor_buf.buf[sensor_buf.curr=(++sensor_buf.curr)%_BUF_SIZE] = result;
}

ISR(ADC_vect)
{
	add_result(ADC);
	adc_start(OPAMP_CHAN);
}

int filter_signal()
{
	int sum = 0;
	for (int i = 0; i < _BUF_SIZE; i++)
		sum += sensor_buf.buf[i];
	return sum/_BUF_SIZE;
}

int main(void) 
{
	int signal = 0;

	DDRB |= 3;
	PORTB |= 1;


	uart_init();
	pwm_init();
	adc_init();
	sei();

	printf("Levitron collider started...\n\r");
	
	while (1) {
		signal = filter_signal();
		printf("adc:%d\n\r", signal);
		_delay_ms(300);		
		TGLBIT(PORTB,0);
	}
	return 0;
}
