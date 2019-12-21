
#ifndef _SBC_H_
#define _SBC_H_

#include <stdint.h>

#include "hw_defs.hpp"
#include "cpu_asm.hpp"
#include "sys_types.hpp"
#include "crt0.hpp"
#include "printf.hpp"
#include "main.hpp"
#include "L1_Peripheral/usb.hpp"
#include "L1_Peripheral/system_controller.hpp"
#include "L1_Peripheral/interrupt_controller.hpp"
#include "L1_Peripheral/pit.hpp"
#include "L1_Peripheral/uart.hpp"
#include "L1_Peripheral/post.hpp"
#include "L3_Application/ring_buf.hpp"
#include "L3_Application/cli.hpp"

#include "comms.hpp"
//#include "flash.h"

#define F_ISR_RXRDY			0x04
#define F_SR_RXRDY			0x01

extern volatile uint8_t __systick_flag;
extern volatile uint8_t __interval_1ms_flag;
extern volatile uint8_t __interval_1us_flag;
extern volatile uint32_t __systick_count;


/* Function prototypes */
void sbc_initialize(void);
void set_post(uint8_t value);
bool get_break(void);



extern SystemController system_controller;
extern Post post;
extern ProgrammableIntervalTimer pit;
extern InterruptController interrupt_controller;


#endif /* _SBC_H_ */
