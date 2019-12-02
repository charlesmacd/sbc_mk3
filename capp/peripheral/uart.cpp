/*
	File:
		uart.h
	Notes:
		None
*/

#include <stdint.h>
#include "uart.hpp"


extern volatile uint8_t *REG_SR;
extern volatile uint8_t *REG_ISR;
extern volatile uint8_t *REG_RHR;
extern volatile uint8_t *REG_IMR;
/* UART (SCC2691) register base */
volatile uint8_t *REG_UART      = (volatile uint8_t *)(UART_BASE);
volatile uint8_t *UART_THR		= (volatile uint8_t *)(UART_BASE + 0x06);
volatile uint8_t *UART_RHR		= (volatile uint8_t *)(UART_BASE + 0x06);
volatile uint8_t *UART_SR 		= (volatile uint8_t *)(UART_BASE + 0x02);


ringbuf_t uart_rx_ringbuf;
ringbuf_t uart_tx_ringbuf;

uint8_t uart_tx_size(void)
{
	return ringbuf_size(&uart_tx_ringbuf);
}

uint8_t uart_rx_count(void)
{
	return ringbuf_size(&uart_rx_ringbuf);
}

uint8_t uart_rx_read(void)
{
	return ringbuf_remove(&uart_rx_ringbuf);
}

/*
	REG_ISR
	[2] = 
	[1] = same as SR.TXEMT
	[0] = same as SR.TXRDY
*/
void uart_tx_write(uint8_t data)
{
	ringbuf_insert(&uart_tx_ringbuf, data);

	/* Rewrite IMR to enable */
	REG_IMR[0] = 0x05;
}

bool uart_keypressed(void)
{
	return (uart_rx_count() == 0) ? false : true;
}

char uart_readkey()
{
	while(!uart_keypressed())
	{
		;
	}
	return uart_rx_read();
}


void uart_sendb(uint8_t data)
{
#if 0
	UART_THR[0] = data;
	while(!(UART_SR[0] & F_UART_TXRDY))
	{
		;
	}
#else	
	uart_tx_write(data);
#endif
}

#if 0

uint8_t uart_getb(void)
{
	while(!(UART_SR[0] & F_UART_RXRDY))
	{
		;
	}
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
	{
		*data++ == uart_getb();
	}
}
#endif

void uart_puts(const char *msg)
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


/* Print 32-bit value as decimal with zero padding */
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
				uart_putch(' ');
			}
		}
	}

	/* Pop digits and print them in order */
	while(index--)
	{
		uart_hexout(stack[index]);
	}
}


/* Print 32-bit value as decimal */
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
	{
		uart_hexout(stack[index]);
	}
}

/* End */
