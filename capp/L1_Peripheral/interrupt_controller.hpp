

#ifndef _INTERRUPT_CONTROLLER_H_
#define _INTERRUPT_CONTROLLER_H_

#include <stdint.h>

extern volatile uint16_t *REG_IPENDING  ;
extern volatile uint16_t *REG_IPEND_CLR ;
extern volatile uint16_t *REG_IMASKED   ;
extern volatile uint16_t *REG_IPEND_SET ;
extern volatile uint16_t *REG_IENABLE   ;
extern volatile uint16_t *REG_IPRIORITY ;

typedef struct 
{
    volatile uint16_t *REG_IPENDING;
    volatile uint16_t *REG_IPEND_CLR;
    volatile uint16_t *REG_IMASKED;
    volatile uint16_t *REG_IPEND_SET;
    volatile uint16_t *REG_IENABLE;
    volatile uint16_t *REG_IPRIORITY;    
} interrupt_controller_register_t;

class InterruptController
{
private:
    interrupt_controller_register_t reg;

public:

    void initialize(void)
    {
        reg.REG_IPENDING  = (volatile uint16_t *)0xFFFFB000; /* R */
        reg.REG_IPEND_CLR = (volatile uint16_t *)0xFFFFB000; /* W */
        reg.REG_IMASKED   = (volatile uint16_t *)0xFFFFB040; /* R */
        reg.REG_IPEND_SET = (volatile uint16_t *)0xFFFFB040; /* W */
        reg.REG_IENABLE   = (volatile uint16_t *)0xFFFFB080; /* R/W */
        reg.REG_IPRIORITY = (volatile uint16_t *)0xFFFFB0C0; /* R */

        /* Disable all interrupt sources */
        set_enable(0x00);

        /* Clear all pending interrupts */
        clear_pending(0xff);
    }

    void set_enable(uint8_t mask)
    {
        uint8_t temp = *reg.REG_IENABLE;
        temp |= mask;
        *reg.REG_IENABLE = temp;
    }

    void clear_enable(uint8_t mask)
    {
        uint8_t temp = *reg.REG_IENABLE;
        temp &= ~mask;
        *reg.REG_IENABLE = temp;
    }

    void clear_pending(uint8_t mask)
    {
        *reg.REG_IPEND_CLR = mask;    
    }

    void force_pending(uint8_t mask)
    {
        *reg.REG_IPEND_SET = mask;
    }

    int8_t get_pending_level(void)
    {
        uint8_t temp = *reg.REG_IPRIORITY;
        if(temp & 0x80)
        {
            return (temp & 0x07);
        }
        
        return -1;
    }
};




#endif /* _INTERRUPT_CONTROLLER_H_ */
