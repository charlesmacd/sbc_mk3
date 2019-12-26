

#include "sbc.hpp"
#include "L0_Platform/cpu_680x0.hpp"
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
cpu_68000 cpu;
SystemController system_controller;
InterruptController interrupt_controller;
Post post;
ProgrammableIntervalTimer pit;
Uart uart;

extern "C" 
{
	extern uint32_t __level1_isr;
	extern uint32_t __level2_isr;
	extern uint32_t __level3_isr;
	extern uint32_t __level4_isr;
	extern uint32_t __level5_isr;
	extern uint32_t __level6_isr;
	extern uint32_t __level7_isr;
};

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

	/* Patch redirection table in RAM that vector table in Flash points to */
	cpu.set_redirected_isr(CPUVectorNumber::AUTOVECTOR_LEVEL1, (IsrHandler)&__level1_isr);
	cpu.set_redirected_isr(CPUVectorNumber::AUTOVECTOR_LEVEL2, (IsrHandler)&__level2_isr);
	cpu.set_redirected_isr(CPUVectorNumber::AUTOVECTOR_LEVEL3, (IsrHandler)&__level3_isr);
	cpu.set_redirected_isr(CPUVectorNumber::AUTOVECTOR_LEVEL4, (IsrHandler)&__level4_isr);
	cpu.set_redirected_isr(CPUVectorNumber::AUTOVECTOR_LEVEL5, (IsrHandler)&__level5_isr);
	cpu.set_redirected_isr(CPUVectorNumber::AUTOVECTOR_LEVEL6, (IsrHandler)&__level6_isr);
	cpu.set_redirected_isr(CPUVectorNumber::AUTOVECTOR_LEVEL7, (IsrHandler)&__level7_isr);

	/* Clear all pending interrupts */
	interrupt_controller.clear_pending(0xFF);

	/* Pick permitted interrupt sources */
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

extern void display_update(void);

void systick_handler(void)
{
	check_comms_dispatch();
}


} // extern c


/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/


