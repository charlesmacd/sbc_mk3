
#ifndef _SYSTEM_CONTROLLER_H_
#define _SYSTEM_CONTROLLER_H_

#ifndef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "../hw_defs.hpp"

#define PERIPHERAL_UART         0

class SystemController
{
    uint32_t peripheral_clock;
    uint32_t cpu_clock;

public:
    void initialize(void)
    {
        peripheral_clock = 8000000;
        cpu_clock = 16000000;        
    }
    
    uint32_t getPeripheralClock(void)
    {
        return peripheral_clock;
    }

    uint32_t getCPUClock(void)
    {
        return cpu_clock;
    }

    void assert_peripheral_reset(int which, bool state)
    {
    	volatile uint8_t *REG_UART_RESET 	= (volatile uint8_t *)(PIO_BASE  + 0x44);
        switch(which)
        {
            case PERIPHERAL_UART:
                REG_UART_RESET[0] = (state) ? 0x01 : 0x00;
                break;
        }
    }
};


#ifndef __cplusplus
}; /* extern */
#endif

#endif /* _PIT_H_ */














