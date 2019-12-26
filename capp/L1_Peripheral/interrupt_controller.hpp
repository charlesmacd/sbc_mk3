

#ifndef _INTERRUPT_CONTROLLER_H_
#define _INTERRUPT_CONTROLLER_H_

#include <stdint.h>

// fix timer
extern volatile uint16_t *REG_IPENDING  ;
extern volatile uint16_t *REG_IPEND_CLR ;
extern volatile uint16_t *REG_IMASKED   ;
extern volatile uint16_t *REG_IPEND_SET ;
extern volatile uint16_t *REG_IENABLE   ;
extern volatile uint16_t *REG_IPRIORITY ;

struct interrupt_controller_register_tag 
{
    union
    {
        struct __attribute__((__packed__))
        {        
			uint8_t padding00[0x01]; 
            uint8_t IPEND_CLR;          /* 0x01.b */
            uint8_t padding01[0x3F];
            uint8_t IPEND_SET;          /* 0x41.b */
            uint8_t padding02[0x3F];
            uint8_t IENABLE;            /* 0x81.b */
            uint8_t padding03[0x3F];
            uint8_t IUNUSED;            /* 0xC1.b */
            uint8_t padding04[0x3E];
        } w; /* Write */
      
        struct  __attribute__((__packed__))
        {        
            uint8_t padding00[0x01];
            uint8_t IPENDING;           /* 0x01.b */
            uint8_t padding01[0x3F];
            uint8_t IMASKED;            /* 0x41.b */
            uint8_t padding02[0x3F];
            uint8_t IENABLE;            /* 0x81.b */
            uint8_t padding03[0x3F];
            uint8_t IPRIORITY;          /* 0xC1.b */
            uint8_t padding04[0x3E];
        } r; /* Read */
    } reg;
};

typedef volatile interrupt_controller_register_tag interrupt_controller_register_t;

class InterruptController
{
private:
    interrupt_controller_register_t *intc;

public:

    void initialize(interrupt_controller_register_t *external_reg = NULL)
    {
        intc = external_reg;
        if(intc == NULL)
        {
            intc = (interrupt_controller_register_t *)0xFFFFB000;
        }

        /* Disable all interrupt sources */
        set_enable(0x00);

        /* Clear all pending interrupts */
        clear_pending(0xff);
    }

    void set_enable(uint8_t mask)
    {
        uint8_t temp = intc->reg.r.IENABLE;
        temp |= mask;
        intc->reg.w.IENABLE = temp;
    }

    void clear_enable(uint8_t mask)
    {
        uint8_t temp = intc->reg.r.IENABLE;
        temp &= ~mask;
        intc->reg.w.IENABLE = temp;
    }

    void clear_pending(uint8_t mask)
    {
        intc->reg.w.IPEND_CLR = mask;    
    }

    void force_pending(uint8_t mask)
    {
        intc->reg.w.IPEND_SET = mask;
    }

    int8_t get_pending_level(void)
    {
        uint8_t temp = intc->reg.r.IPRIORITY;
        if(temp & 0x80)
        {
            return (temp & 0x07);
        }
        
        return -1;
    }
};




#endif /* _INTERRUPT_CONTROLLER_H_ */
