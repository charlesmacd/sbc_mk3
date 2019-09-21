

#include <stdint.h>
#include "uart.h"



/* Hardware registers */
volatile uint8_t *UART_THR		= (volatile uint8_t *)(UART_BASE + 0x06);
volatile uint8_t *UART_RHR		= (volatile uint8_t *)(UART_BASE + 0x06);
volatile uint8_t *UART_SR 		= (volatile uint8_t *)(UART_BASE + 0x02);


/* UART (SCC2691) register base */
volatile uint8_t *REG_UART      = (volatile uint8_t *)(UART_BASE);



void uart_sendb(uint8_t data)
{
	UART_THR[0] = data;
	while(!(UART_SR[0] & F_UART_TXRDY))
		;
}

uint8_t uart_getb(void)
{
	while(!(UART_SR[0] & F_UART_RXRDY))
		;
	return UART_RHR[0];
}

void uart_send(uint8_t *data, uint32_t size)
{
	while(size--)
		uart_sendb(*data++);
}

void uart_get(uint8_t *data, uint32_t size)
{
	while(size--)
		*data++ == uart_getb();
}


#if 0

bool keypressed(void)
{
	return (UART_SR[0] & F_UART_RXRDY) ? true : false;
}
#endif
void uart_puts(char *msg)
{
	for(int i = 0; msg[i] != 0; i++)
	{
		uart_putch(msg[i]);
	}
}

void uart_hexout(uint8_t digit)
{
	uart_putch("0123456789ABCDEF"[digit & 0x0F]);
}

void uart_printhexb(uint8_t value)
{
	uart_hexout((value >> 4) & 0x0F);
	uart_hexout((value >> 0) & 0x0F);
}

void uart_printhexw(uint16_t value)
{
	uart_printhexb(	(value >> 8) & 0xFF);
	uart_printhexb((value >> 0) & 0xFF);
}

void uart_printhexl(uint32_t value)
{
	uart_printhexw((value >> 16) & 0xFFFF);
	uart_printhexw((value >> 0 ) & 0xFFFF);
}

void uart_printd_padding(uint32_t value, uint8_t padding_width, uint8_t padding_type)
{
	uint8_t stack[10];
	uint8_t index = 0; 

	/* Push decimal digits back-to-front */
	do {
		stack[index++] = (value % 10);
		value /= 10;		
	} while(value);

	
	/* Print leading zeros */
	if(padding_width)
	{
		for(int k = index; k < padding_width; k++)
		{
			if(padding_type == 0)
				uart_hexout(0);
			else
			{
					uart_sendb(' ');
			}
		}
	}

	/* Pop digits and print them in order */
	while(index--)
		uart_hexout(stack[index]);
}


void uart_printd(uint32_t value)
{
	uint8_t stack[10];
	uint8_t index = 0; 

	/* Push decimal digits back-to-front */
	do {
		stack[index++] = (value % 10);
		value /= 10;		
	} while(value);

	/* Pop digits and print them in order */
	while(index--)
		uart_hexout(stack[index]);
}
