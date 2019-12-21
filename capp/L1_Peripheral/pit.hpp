
#ifndef _PIT_H_
#define _PIT_H_

#ifndef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "../hw_defs.hpp"

#define PIT_CNT_EVENT		                0x00
#define PIT_CNT_HW_1SHOT	                0x01
#define PIT_CNT_REPEAT		                0x02
#define PIT_CNT_SQW			                0x03
#define PIT_CNT_SW_TRG		                0x04
#define PIT_CNT_HW_TRG		                0x05

#define PIT_CNT_BINARY		                0x00
#define PIT_CNT_BCD			                0x01

#define PIT_LOAD_LOW		                0x00
#define PIT_LOAD_HIGH		                0x10
#define PIT_LOAD_WORD		                0x30

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

#define PIT_CTRL(counter, mode, type)	(counter << 3) | (mode << 1) | (type)

typedef struct 
{
    volatile uint16_t *DATA;
} pit_register_t;

class ProgrammableIntervalTimer
{
private:
    pit_register_t reg;

public:

    /* Initialize local state */
    void initialize(void)
    {
        reg.DATA = (volatile uint16_t *)WORD_ADDRESS(PIT_BASE);
    }

    /* Write to PIT register */
    void write(uint8_t offset, uint16_t value)
    {
        reg.DATA[offset & 0x03] = value;
    }

    /* Read from PIT register */
    uint8_t read(uint8_t offset)
    {
        return reg.DATA[offset & 0x03];
    }

    uint8_t format_control_word(uint8_t select_counter, uint8_t access_mode, uint8_t counter_mode, bool is_bcd)
    {
        return (select_counter & 3) << 6
             | (access_mode & 3) << 4
             | (counter_mode & 7) << 1
             | (is_bcd & 1)
             ;
    }


    void write_control_word(uint8_t counter, uint8_t access_mode, uint16_t counter_mode, bool is_bcd)
    {
        write(3, format_control_word(counter, access_mode, counter_mode, is_bcd));        
    }

    void write_counter(uint8_t channel, uint16_t count)
    {
        /* Write low byte of count */
        write(channel, (count >> 0) & 0xFF);

        /* Write high byte of count */
        write(channel, (count >> 8) & 0xFF);
    }

    uint16_t read_counter(uint8_t channel)
    {
        uint16_t count;
        
        /* Write control word with D[7:6]=Channel, D[5:0]=0 */
        write(3, (channel & 3) << 6);
        
        /* Read low byte of result */
        count = read(channel);
        
        /* Read high byte of result */
        count = (read(channel) << 8) | (count & 0x00FF);

        return count;
    }
};



#ifndef __cplusplus
}; /* extern */
#endif

#endif /* _PIT_H_ */