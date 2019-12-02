/*
	File:
		timer.cpp
	Author:
		Charles MacDonald
	Notes:
		None
*/

#include "sbc.hpp"
#include "timer.hpp"
#include "peripheral/interrupt_controller.hpp"

extern volatile uint8_t *REG_TIMER_1US_ENABLE;
extern volatile uint8_t *REG_TIMER_1MS_ENABLE;

#define PIT_ACCESS_LATCH					0
#define PIT_ACCESS_LSB						1
#define PIT_ACCESS_MSB						2
#define PIT_ACCESS_SEQUENTIAL				3
#define PIT_COUNTER_MODE_IOTC				0
#define PIT_COUNTER_MODE_HW1S				1
#define PIT_COUNTER_MODE_RATE_GENERATOR		2
#define PIT_COUNTER_MODE_SQUARE_WAVE		3
#define PIT_COUNTER_MODE_SW_TRG				4
#define PIT_COUNTER_MODE_HW_TRG				5

/* Format control word fields */
uint8_t pit_format_control_word(uint8_t select_counter, uint8_t access_mode, uint8_t counter_mode, bool is_bcd)
{
	return (select_counter & 3) << 6
	     | (access_mode & 3) << 4
		 | (counter_mode & 7) << 1
		 | (is_bcd & 1)
		 ;
}

/* Write control word to configure a counter */
void pit_write_control_word(uint8_t counter, uint8_t access_mode, uint16_t counter_mode, bool is_bcd)
{
	REG_PIT[3] = pit_format_control_word(counter, access_mode, counter_mode, is_bcd);
}

/* Write 16-bit timer count */
void pit_write_counter(uint8_t channel, uint16_t count)
{
	/* Write low byte of count */
	REG_PIT[channel] = (count >> 0) & 0xFF;

	/* Write high byte of count */
	REG_PIT[channel] = (count >> 8) & 0xFF;
}

/* Read 16-bit timer count */
uint16_t pit_read_counter(uint8_t channel)
{
	uint16_t count;
	
	/* Write control word with D[7:6]=Channel, D[5:0]=0 */
	REG_PIT[3] = (channel & 3) << 6;	
	
	/* Read low byte of result */
	count = REG_PIT[channel];
	
	/* Read high byte of result */
	count = (REG_PIT[channel] << 8) | count;	

	return count;
}


void set_1ms_timer(uint16_t count)
{
	timer_1ms_enable(false);
	pit_write_control_word(2, PIT_ACCESS_SEQUENTIAL, PIT_COUNTER_MODE_SW_TRG, false);
	pit_write_counter(2, count);
	timer_1ms_enable(true);
	__interval_1ms_flag = 0;

#if 1
	REG_IPEND_CLR[0] = 0x10;
	REG_IENABLE[0] |= 0x10;
#else
	intc_acknowledge(0x10);
	intc_modify_enable(0x00, 0x10);
#endif
}

void set_1us_timer(uint16_t count)
{
	timer_1us_enable(false);
	pit_write_control_word(0, PIT_ACCESS_SEQUENTIAL, PIT_COUNTER_MODE_SW_TRG, false);
	pit_write_counter(0, count);
	timer_1us_enable(true);
	__interval_1us_flag = 0;
#if 1
	REG_IPEND_CLR[0] = 0x20;
	REG_IENABLE[0] |= 0x20;
#else	
	intc_acknowledge(0x20);
	intc_modify_enable(0x00, 0x20);
#endif	
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
	timer_1us_enable(false);
	timer_1ms_enable(false);

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
	pit_write_control_word(1, PIT_ACCESS_SEQUENTIAL, PIT_COUNTER_MODE_RATE_GENERATOR, true);
	pit_write_counter(1, 0x1000);
}

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

void delay_ms(uint16_t duration)
{
	set_1ms_timer(duration - 2);
	while(__interval_1ms_flag == 0)
	{
		; /* Block until timer expires */
	}
}


