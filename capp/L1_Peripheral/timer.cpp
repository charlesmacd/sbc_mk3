/*
	File:
		timer.cpp
	Author:
		Charles MacDonald
	Notes:
		None


	PIT configuration for timers:

	Channel 1
	CLK0  - X1M
	GATE0 - VCC						Always enabled
	OUT0  - SYSTICK_1MS				To CPLD, becomes source for Level 6 interrupt

	Channel 0
	CLK0  - X1M
	GATE0 - INTERVAL_1US_ENA		Controlled via CPLD register bit
	OUT0  - INTERVAL_1US			To CPLD, becomes source for Level 5 interrupt

	Channel 2
	CLK0  - SYSTICK_1MS
	GATE0 - INTERVAL_1MS_ENA		Controlled via CPLD register bit
	OUT0  - INTERVAL_1MS			To CPLD, becomes source for Level 4 interrupt
*/

/*
	measuring pin 17
	OUT2 (interval_1ms)

	When count=10
	Low for 999.99us
	High for 10.999us
	Total is 11.9999us

	When count=9
	Low for 999.99us
	High for 10.999us
	Total is 11.9999us

	When count=9
	Low for 1ms
	High for 9.99ms
	Total is 10.9999ms

	sw trg
	Initially H, out goes L for one clock pulse (1 clock pulse is 1ms)

	
*/
	/* Program system tick timer 
	   NOTE:
	   CLK1 = X1M
	   G1   = VCC (always counting)
	   OUT1 = SYSTICK_MS
	   Mode = Rate generator
	   Count = 1000 (BCD)
	   Measured count from negative edge of SYSTICK_MS to next negative edge:
	   - 0x0999 :  998.99us
	   - 0x1000 :  999.99us
	   - 0x1001 : 1000.98us
	*/


#include "../sbc.hpp"
#include "timer.hpp"
#include "pit.hpp"
#include "interrupt_controller.hpp"

volatile uint8_t *REG_TIMER_1US_ENABLE 	= (volatile uint8_t *)(PIO_BASE  + 0x40);
volatile uint8_t *REG_TIMER_1MS_ENABLE 	= (volatile uint8_t *)(PIO_BASE  + 0x42);

void set_1ms_timer(uint16_t count)
{
	interrupt_controller.clear_enable(0x10);

	timer_1ms_enable(false);
	pit.write_control_word(2, PIT_ACCESS_SEQUENTIAL, PIT_COUNTER_MODE_SW_TRG, false);
	pit.write_counter(2, count);
	timer_1ms_enable(true);
	__interval_1ms_flag = 0;

	interrupt_controller.clear_pending(0x10);
	interrupt_controller.set_enable(0x10);
}

void set_1us_timer(uint16_t count)
{
	interrupt_controller.clear_enable(0x20);

	timer_1us_enable(false);
	pit.write_control_word(0, PIT_ACCESS_SEQUENTIAL, PIT_COUNTER_MODE_SW_TRG, false);
	pit.write_counter(0, count);
	timer_1us_enable(true);
	__interval_1us_flag = 0;

	interrupt_controller.clear_pending(0x20);
	interrupt_controller.set_enable(0x20);
}

void timer_1ms_enable(bool enabled)
{
	REG_TIMER_1MS_ENABLE[0] = (enabled) ? 1 : 0;
}

void timer_1us_enable(bool enabled)
{
	REG_TIMER_1US_ENABLE[0] = (enabled) ? 1 : 0;
}

void timer_init(void)
{
	const int kSystickIntervalBCD 	= 0x1000; /* 1ms */
	const int kSystickChannel       = 1;
	const bool kSystickBCDCountMode = true;

	/* Disable user timers */
	timer_1us_enable(false);
	timer_1ms_enable(false);

	/* Configure system tick timer */
	pit.write_control_word(
		kSystickChannel, 
		PIT_ACCESS_SEQUENTIAL, 
		PIT_COUNTER_MODE_RATE_GENERATOR, 
		kSystickBCDCountMode
		);
	pit.write_counter(kSystickChannel, kSystickIntervalBCD);
}

void delay_ms(uint16_t duration)
{
	set_1ms_timer(duration - 2);
	while(__interval_1ms_flag == 0)
	{
		; /* Block until timer expires */
	}
}


/* End */