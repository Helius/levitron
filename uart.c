#include <stdio.h>
#include <avr/io.h>

#include "uart.h"

#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)

int uart_putchar(char c, FILE *stream) {
	while(!(UCSRA & (1<<UDRE)));
	UDR = c;
	return 0;
}

int uart_getchar(FILE *sttream) {
	while(!(UCSRA & (1<<RXC)));
	return UDR;
}

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void uart_init(void) {

	UBRRH = (uint8_t)(BAUD_PRESCALLER>>8);
	UBRRL = (uint8_t)(BAUD_PRESCALLER);
	UCSRB = (1<<RXEN)|(1<<TXEN);
	UCSRC = ((1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1));

	stdout = &uart_output;
	stdin  = &uart_input;
}

