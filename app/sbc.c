
#include "sbc.h"

extern volatile uint8_t *REG_STATUS;
extern volatile uint8_t *REG_POST;

/* USB (UM245) data register */
volatile uint8_t *USB_DATA		= (volatile uint8_t *)(USB_BASE);

/* I2C (PCF8584) register base */
volatile uint8_t *REG_I2C       = (volatile uint8_t *)(I2C_BASE);

/* POST (TIL311) register base */
volatile uint8_t *REG_POST		= (volatile uint8_t *)(POST_BASE);

/* HI status register (EPM7128) */
volatile uint8_t *REG_STATUS 	= (volatile uint8_t *)(PIO_BASE  + 0x40);

/* PIO registers */
volatile uint8_t *REG_TIMER_1US_ENABLE 	= (volatile uint8_t *)(PIO_BASE  + 0x40);
volatile uint8_t *REG_TIMER_1MS_ENABLE 	= (volatile uint8_t *)(PIO_BASE  + 0x42);

/* UART registers */
volatile uint8_t *REG_SR  		= (volatile uint8_t *)(UART_BASE + 0x02);
volatile uint8_t *REG_ISR 		= (volatile uint8_t *)(UART_BASE + 0x0A);
volatile uint8_t *REG_RHR 		= (volatile uint8_t *)(UART_BASE + 0x06);
volatile uint8_t *REG_IMR		= (volatile uint8_t *)(UART_BASE + 0x0A);

volatile uint16_t *REG_USER_LED  = (volatile uint16_t *)0xFFFF8046;
volatile uint16_t *REG_IPENDING  = (volatile uint16_t *)0xFFFFB000;
volatile uint16_t *REG_IPEND_CLR = (volatile uint16_t *)0xFFFFB000;
volatile uint16_t *REG_IPEND_SET = (volatile uint16_t *)0xFFFFB040;
volatile uint16_t *REG_IENABLE   = (volatile uint16_t *)0xFFFFB080;
volatile uint16_t *REG_IMASKED   = (volatile uint16_t *)0xFFFFB040;
volatile uint16_t *REG_IPRIORITY = (volatile uint16_t *)0xFFFFB0C0;

volatile uint16_t *REG_STATUS0 	= (volatile uint16_t *)0xFFFF8000;
volatile uint16_t *REG_STATUS1 	= (volatile uint16_t *)0xFFFF8040;
volatile uint16_t *REG_STATUS2 	= (volatile uint16_t *)0xFFFF8080;
volatile uint16_t *REG_STATUS3 	= (volatile uint16_t *)0xFFFF80C0;

volatile uint16_t *REG_PIT = (volatile uint16_t *)WORD_ADDRESS(PIT_BASE);

#define F_ISR_RXRDY			0x04
#define F_SR_RXRDY			0x01
#define WORD_ADDRESS(x)		(x&~1)


uint8_t __systick_flag;
uint8_t __interval_1ms_flag;
uint8_t __interval_1us_flag;

ringbuf_t uart_rx_ringbuf;
ringbuf_t uart_tx_ringbuf;


void set_post(uint8_t value)
{
	REG_POST[0] = value;
}

bool get_break(void)
{
	return (REG_STATUS[0] & F_STATUS_BREAK) ? false : true;
}


void write_output_latch(int which, int offset, int state)
{
	if(which)
	{

	}
	else
	{

	}
	
}




/*--------------------------------------------------------------------*/
/* Interrupt handlers */
/*--------------------------------------------------------------------*/

// __interval_1us_flag is set by isr
void interval_1us_handler(void)
{
	// called by isr
}

// __interval_1ms_flag is set by isr
void interval_1ms_handler(void)
{
	// called by isr
}

void systick_handler(void)
{
	check_comms_dispatch();
}





/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

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

void uart_tx_write(uint8_t data)
{
	ringbuf_insert(&uart_tx_ringbuf, data);
	REG_IMR[0] = 0x05;
}

bool uart_keypressed(void)
{
	return uart_rx_count();
}

char uart_readkey()
{
	while(!uart_keypressed())
	{
		;
	}
	return uart_rx_read();
}



