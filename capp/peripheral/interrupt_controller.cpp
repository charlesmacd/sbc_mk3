/*
    Interrupt controller
    Register functions

    PENDING  : Read to input pending state flip-flop.
    PEND_CLR : Write '1' to clear pending state flip-flop.
    PEND_SET : Write '1' to set pending state flip-flop.
               This is a debugging feature to force an interrupt via software.
    ENABLE   : Write '1' to enable an interrupt source to set a flip-flop
    MASKED   : Read to input pending state flip-flop as masked by enable register
    PRIORITY : Read to input number of highest set flip-flop:
               D7     : '1' if any flip-flop is set from PENDING & MASKED.
               D[2:0] : Flip-flop number.

    Register map

    Read
    0 - PENDING
    1 - MASKED   (= PENDING & ENABLE)
    2 - ENABLE
    3 - PRIORITY [GS,0,0,0,0,PR2,PR1,PR0]

    Write
    0 - PENDING CLR
    1 - PENDING SET
    2 - ENABLE
    3 - (Not used)

*/

#include <stdio.h>
#include <stdint.h>
#include "interrupt_controller.hpp"

volatile uint16_t *REG_IPENDING  = (volatile uint16_t *)0xFFFFB000; /* R */
volatile uint16_t *REG_IPEND_CLR = (volatile uint16_t *)0xFFFFB000; /* W */
volatile uint16_t *REG_IMASKED   = (volatile uint16_t *)0xFFFFB040; /* R */
volatile uint16_t *REG_IPEND_SET = (volatile uint16_t *)0xFFFFB040; /* W */
volatile uint16_t *REG_IENABLE   = (volatile uint16_t *)0xFFFFB080; /* R/W */
volatile uint16_t *REG_IPRIORITY = (volatile uint16_t *)0xFFFFB0C0; /* R */

void intc_modify_enable(uint8_t clear, uint8_t set)
{
    uint8_t temp = *REG_IENABLE;
    temp &= ~clear;
    temp |= set;
    *REG_IENABLE = temp;
}

void intc_enable(uint8_t mask)
{
    *REG_IENABLE = mask;
}

void intc_acknowledge(uint8_t mask)
{
    *REG_IPEND_CLR = mask;
}

void intc_force_pending(uint8_t mask)
{
    *REG_IPEND_SET = mask;
}

uint8_t intc_get_pending_priority(void)
{
    uint8_t temp = *REG_IPRIORITY;
    return temp & 7;
}

uint8_t intc_get_pending(void)
{
    return *REG_IPENDING;
}


/* End */