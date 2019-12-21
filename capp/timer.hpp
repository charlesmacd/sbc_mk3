
#ifndef _TIMER_H_
#define _TIMER_H_

#ifndef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hw_defs.hpp"

#define kUsTimerMask          0x20
#define kMsTimerMask          0x10

typedef struct
{
    volatile uint8_t *TMR_1US_ENABLE;
    volatile uint8_t *TMR_1MS_ENABLE;
} timer_register_t;

class Timer
{
public:
    timer_register_t reg;

    void initialize(void)
    {
        reg.TMR_1US_ENABLE = (volatile uint8_t *)(PIO_BASE + 0x0040);
        reg.TMR_1MS_ENABLE = (volatile uint8_t *)(PIO_BASE + 0x0042);
    }

    
    void delay_ms(uint16_t duration)
    {
        uint32_t limit = __systick_count + duration;
        while(__systick_count < limit)
        {
            ;
        }

    }

    void delay_us(uint16_t duration)
    {
    }

    void enable_ms_timer(bool is_enabled)
    {
        *reg.TMR_1MS_ENABLE = (is_enabled) ? 0x01 : 0x00;
    }

    void enable_us_timer(bool is_enabled)
    {
        *reg.TMR_1US_ENABLE = (is_enabled) ? 0x01 : 0x00;
    }

    void trigger_interrupt_in_ms(uint16_t count)
    {
        const int kMsTimerChannel = 2;
        const bool kMsTimerCountBinary = true;

        /* Stop timer from running */
        enable_ms_timer(false);

        /* Set counting mode (software triggered one-shot, binary count) */
        pit.write_control_word(
            kMsTimerChannel, 
            PIT_ACCESS_SEQUENTIAL, 
            PIT_COUNTER_MODE_SW_TRG, 
            kMsTimerCountBinary
            );

        /* Write new terminal count */
        pit.write_counter(kMsTimerChannel, count);

        /* Clear interrupt occurance flag that interrupt handler sets */
        __interval_1ms_flag = 0;

        // use system controller's pointer to interrupt controller

        interrupt_controller.clear_pending(kMsTimerMask);
        interrupt_controller.set_enable(kMsTimerMask);

        /* Start counting */
        enable_ms_timer(true);
    }

    void triger_interrupt_in_us(uint16_t count)
    {
        
    }
};




/* Function prototypes */
void set_1ms_timer(uint16_t count);
void set_1us_timer(uint16_t count);
void timer_1ms_enable(bool enabled);
void timer_1us_enable(bool enabled);
void timer_init(void);
void delay_ms(uint16_t duration);


#ifndef __cplusplus
}; /* extern */
#endif

#endif /* _TIMER_H_ */






