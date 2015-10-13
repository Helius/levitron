#include <avr/io.h>
//#include <avr/wdt.h>
//#include <avr/interrupt.h>
#include <util/delay.h>
//#include <avr/eeprom.h>
//#include <avr/sleep.h>
//#include <avr/pgmspace.h>
//#include <string.h>

#include "uart.h"

#define FALSE         0
#define TRUE          1

#define TGLBIT(REG, BIT)   (REG ^= (1 << BIT))
#define CLRBIT(REG, BIT)   (REG &= ~(1 << BIT))
#define SETBIT(REG, BIT)   (REG |= (1 << BIT))
#define TSTBIT(REG, BIT)   (REG & (1 << BIT))



int main(void) 
{
// configure PB0 for output and blink
	DDRB = 1;
	PORTB = 1;

// setup uart
	uart_init();

	printf("Levitron collider started...\n\r");

	while (1) {
		printf("do work...\n\r");
		_delay_ms(500);		
		TGLBIT(PORTB,0);
	}
	return 0;
}
