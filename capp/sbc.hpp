
#ifndef _SBC_H_
#define _SBC_H_

#include <stdint.h>

#include "cpu_asm.hpp"
#include "sys_types.hpp"
#include "crt0.hpp"
#include "printf.hpp"
#include "main.hpp"
#include "peripheral/usb.hpp"
#include "peripheral/interrupt_controller.hpp"
#include "comms.hpp"
//#include "flash.h"
#include "peripheral/uart.hpp"
#include "peripheral/post.hpp"
#include "utility/ringbuf.h"
#include "application/cli.hpp"

#include "hw_defs.hpp"

#if 0
extern volatile uint8_t *USB_DATA;
extern volatile uint8_t *REG_I2C;
extern volatile uint8_t *REG_POST;
extern volatile uint8_t *REG_STATUS;
extern volatile uint8_t *REG_TIMER_1US_ENABLE;
extern volatile uint8_t *REG_TIMER_1MS_ENABLE;
extern volatile uint16_t *REG_USER_LED;
extern volatile uint16_t *REG_IPENDING;
extern volatile uint16_t *REG_IPEND_CLR;
extern volatile uint16_t *REG_IPEND_SET;
extern volatile uint16_t *REG_IENABLE;
extern volatile uint16_t *REG_IMASKED;
extern volatile uint16_t *REG_IPRIORITY;
extern volatile uint16_t *REG_STATUS0;
extern volatile uint16_t *REG_STATUS1;
extern volatile uint16_t *REG_STATUS2;
extern volatile uint16_t *REG_STATUS3;
extern volatile uint8_t *REG_SR;
extern volatile uint8_t *REG_ISR;
extern volatile uint8_t *REG_RHR;
extern volatile uint8_t *REG_IMR;
#endif

#define F_ISR_RXRDY			0x04
#define F_SR_RXRDY			0x01
#define WORD_ADDRESS(x)		(x&~1)

extern volatile uint16_t *REG_PIT;

extern volatile uint8_t __systick_flag;
extern volatile uint8_t __interval_1ms_flag;
extern volatile uint8_t __interval_1us_flag;
extern volatile uint32_t __systick_count;


/* Function prototypes */
void sbc_initialize(void);
void set_post(uint8_t value);
bool get_break(void);

extern Post post;
extern InterruptController interrupt_controller;

#endif /* _SBC_H_ */
