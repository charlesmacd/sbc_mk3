

#ifndef _INTERRUPT_CONTROLLER_H_
#define _INTERRUPT_CONTROLLER_H_

#include <stdint.h>

extern volatile uint16_t *REG_IPENDING;
extern volatile uint16_t *REG_IPEND_CLR;
extern volatile uint16_t *REG_IMASKED;
extern volatile uint16_t *REG_IPEND_SET;
extern volatile uint16_t *REG_IENABLE;
extern volatile uint16_t *REG_IPRIORITY;

typedef struct 
{
    
} interrupt_controller_register_t;


class InterruptController
{
public:
    void initialize(void)
    {
        /* Disable all interrupt sources */
        enable(0x00);

        /* Clear all pending interrupts */
        clear_pending(0xff);
    }

    void enable(uint8_t mask)
    {
        *REG_IENABLE = mask;
    }

    void clear_pending(uint8_t mask)
    {
        *REG_IPEND_CLR = mask;    
    }

    void force_pending(uint8_t mask)
    {
        *REG_IPEND_SET = mask;
    }

    int8_t get_pending_level(void)
    {
        uint8_t temp = *REG_IPRIORITY;
        if(temp & 0x80)
        {
            return (temp & 0x07);
        }
        
        return -1;
    }
};




/* Function prototypes */
void intc_modify_enable(uint8_t clear, uint8_t set);
void intc_enable(uint8_t mask);
void intc_acknowledge(uint8_t mask);
void intc_force_pending(uint8_t mask);
uint8_t intc_get_pending_priority(void);
uint8_t intc_get_pending(void);

#endif /* _INTERRUPT_CONTROLLER_H_ */
