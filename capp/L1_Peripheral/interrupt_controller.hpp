

#ifndef _INTERRUPT_CONTROLLER_H_
#define _INTERRUPT_CONTROLLER_H_

#include <stdint.h>
#include "..\sys_types.hpp"


/* Board specific address */
constexpr uint32_t kInterruptControllerRegBase = 0xFFFFB000;

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/


struct interrupt_controller_register_tag
{
	union
	{
		struct
		{
			/* 0x00 */
			__OM uint16_t IPEND_CLR;
			STRUCT_PADDING(0, 0x40, 1);

			/* 0x40 */
            __OM uint16_t IPEND_SET;
			STRUCT_PADDING(1, 0x40, 1);

			/* 0x80 */
            __OM uint16_t IENABLE;
			STRUCT_PADDING(2, 0x40, 1);

			/* 0xC0 */
            __OM uint16_t IUNUSED;
			STRUCT_PADDING(3, 0x40, 1);
		} w;

		struct
		{
			/* 0x00 */
			__IM uint16_t IPENDING;
			STRUCT_PADDING(0, 0x40, 1);

			/* 0x40 */
			__IM uint16_t IMASKED;
			STRUCT_PADDING(1, 0x40, 1);

			/* 0x80 */
			__IM uint16_t IENABLE;
			STRUCT_PADDING(2, 0x40, 1);

			/* 0xC0 */
			__IM uint16_t IPRIORITY;
			STRUCT_PADDING(3, 0x40, 1);
		} r;	
	} reg;
} __attribute__((packed, aligned(2)));

typedef interrupt_controller_register_tag interrupt_controller_register_t;

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/


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
            intc = (interrupt_controller_register_t *)kInterruptControllerRegBase;
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
