
#include "sbc.h"
#include "timer.h"



void pit_write_counter(uint8_t channel, uint16_t count)
{
	/* Write low byte of count */
	REG_PIT[channel] = (count >> 0) & 0xFF;

	/* Write high byte of count */
	REG_PIT[channel] = (count >> 8) & 0xFF;
}

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
	__interval_1ms_flag = 0;
	REG_TIMER_1MS_ENABLE[0] = 0x00;
	REG_PIT[3] = 0xB8; 
	REG_PIT[2] = (count >> 0) & 0xFF;
	REG_PIT[2] = (count >> 8) & 0xFF;
	REG_TIMER_1MS_ENABLE[0] = 0x01;
	REG_IPEND_CLR[0] = 0x10;
	REG_IENABLE[0] |= 0x10;
}

void set_1us_timer(uint16_t count)
{
	__interval_1us_flag = 0;
	REG_TIMER_1US_ENABLE[0] = 0x00;
	REG_PIT[3] = 0x38;
	REG_PIT[0] = (count >> 0) & 0xFF;
	REG_PIT[0] = (count >> 8) & 0xFF;
	REG_TIMER_1US_ENABLE[0] = 0x01;
	REG_IPEND_CLR[0] = 0x20;
	REG_IENABLE[0] |= 0x20;
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
	REG_PIT[3] = 0x75;
	REG_PIT[1] = 0x00;
	REG_PIT[1] = 0x10;
}



void delay_ms(uint16_t duration)
{
	set_1ms_timer(duration);

	while(__interval_1ms_flag == 0)
	{
		; /* Block until timer expires */
	}
}


