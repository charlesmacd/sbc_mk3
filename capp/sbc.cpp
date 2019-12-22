

#include "sbc.hpp"
#include "L1_Peripheral/pit.hpp"
#include "L1_Peripheral/post.hpp"
#include "L1_Peripheral/uart.hpp"
#include "L1_Peripheral/system_controller.hpp"
#include "L1_Peripheral/interrupt_controller.hpp"
#include "timer.hpp"

volatile uint8_t *REG_STATUS 	= (volatile uint8_t *)(PIO_BASE  + 0x40);

#define F_STATUS_BREAK		0x01

#define F_ISR_RXRDY			0x04
#define F_SR_RXRDY			0x01
#define WORD_ADDRESS(x)		(x&~1)


volatile uint8_t __systick_flag;
volatile uint8_t __interval_1ms_flag;
volatile uint8_t __interval_1us_flag;
volatile uint32_t __systick_count;

/* Global classes */
SystemController system_controller;
InterruptController interrupt_controller;
Post post;
ProgrammableIntervalTimer pit;
Uart uart;


void sbc_initialize(void)
{
	/* Set up system controller */
	system_controller.initialize();

	/* Set up 82C55 PIT */
	pit.initialize();

	/* Set up TIL311 */
	post.initialize();

	/* Set up interrupt controller */
	interrupt_controller.initialize();

	/* Set up timer hardware */
	timer_init();

	/* Set up UART */
	uart.initialize();

	/* Clear all pending interrupts */
	interrupt_controller.clear_pending(0xFF);

	/* Enable interrupt levels 7, 6, 3 */

	// Level 7 : Break (NMI)
	// Level 6 : System tick
	// Level 5 : US timer
	// Level 4 : MS timer
	// Level 3 : UART
	// Level 2 : Unused
	// Level 1 : Unused
	interrupt_controller.set_enable(0xF8);
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


