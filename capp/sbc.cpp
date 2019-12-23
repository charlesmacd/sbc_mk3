

#include "sbc.hpp"
#include "L1_Peripheral/pit.hpp"
#include "L1_Peripheral/post.hpp"
#include "L1_Peripheral/uart.hpp"
#include "L1_Peripheral/system_controller.hpp"
#include "L1_Peripheral/interrupt_controller.hpp"
#include "timer.hpp"

/* Global variables modified by ISRs */
volatile uint8_t __systick_flag;
volatile uint8_t __interval_1ms_flag;
volatile uint8_t __interval_1us_flag;
volatile uint32_t __systick_count;

/* Global objects */
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
	interrupt_controller.set_enable(
		INTCON_LV7_BRK     |
		INTCON_LV6_SYSTICK |
		INTCON_LV5_USTIMER |
		INTCON_LV4_MSTIMER |
		INTCON_LV3_UART    
		);
}


bool get_break(void)
{
	const uint8_t kStatusBreak = 0x01;
	volatile uint8_t *REG_STATUS = (volatile uint8_t *)(PIO_BASE + 0x40);
	return (REG_STATUS[0] & kStatusBreak) ? false : true;
}



/*--------------------------------------------------------------------*/
/* Interrupt handlers */
/*--------------------------------------------------------------------*/

extern "C" {

void interval_1us_handler(void)
{
	;
}

void interval_1ms_handler(void)
{
	;
}

void systick_handler(void)
{
	check_comms_dispatch();
}


}; // extern c


/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/


