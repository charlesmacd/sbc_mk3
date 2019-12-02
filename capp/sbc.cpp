

#include "sbc.hpp"
#include "peripheral/post.hpp"
#include "peripheral/interrupt_controller.hpp"
#include "timer.hpp"

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

volatile uint16_t *REG_STATUS0 	= (volatile uint16_t *)0xFFFF8000;
volatile uint16_t *REG_STATUS1 	= (volatile uint16_t *)0xFFFF8040;
volatile uint16_t *REG_STATUS2 	= (volatile uint16_t *)0xFFFF8080;
volatile uint16_t *REG_STATUS3 	= (volatile uint16_t *)0xFFFF80C0;

volatile uint16_t *REG_PIT = (volatile uint16_t *)WORD_ADDRESS(PIT_BASE);

#define F_ISR_RXRDY			0x04
#define F_SR_RXRDY			0x01
#define WORD_ADDRESS(x)		(x&~1)


volatile uint8_t __systick_flag;
volatile uint8_t __interval_1ms_flag;
volatile uint8_t __interval_1us_flag;
volatile uint32_t __systick_count;

/* Global classes */
InterruptController interrupt_controller;
Post post;

void sbc_initialize(void)
{
	/* Set up TIL311 */
	post.initialize();

	/* Set up interrupt controller */
	interrupt_controller.initialize();

	/* Set up PIT 82C54 */
	timer_init();

	/* Clear all pending interrupts */
	intc_acknowledge(0xFF);

	/* Enable interrupt levels 7, 6, 3 */
	intc_enable(0xC8);

	/* Initialize TX/RX ring buffers */
	ringbuf_init(&uart_rx_ringbuf, RINGBUF_MAX_SIZE);
	ringbuf_init(&uart_tx_ringbuf, RINGBUF_MAX_SIZE);


}



bool get_break(void)
{
	return (REG_STATUS[0] & F_STATUS_BREAK) ? false : true;
}



/*--------------------------------------------------------------------*/
/* Interrupt handlers */
/*--------------------------------------------------------------------*/

extern "C" {

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


}; // extern c


/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/


