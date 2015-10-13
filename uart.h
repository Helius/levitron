#ifndef _uart_h
#define _uart_h 1

#include <avr/io.h>
#include <stdio.h>

void uart_init(void);
int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *sttream);

#endif /* defined _uart_h */
