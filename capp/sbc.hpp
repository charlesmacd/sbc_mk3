
#ifndef _SBC_H_
#define _SBC_H_

#include <stdint.h>
#include "hw_defs.hpp"
#include "sys_types.hpp"
#include "newlib/printf.hpp"
#include "main.hpp"
#include "crt0.hpp"
#include "L0_Platform/cpu_680x0.hpp"
#include "L1_Peripheral/usb.hpp"
#include "L1_Peripheral/system_controller.hpp"
#include "L1_Peripheral/interrupt_controller.hpp"
#include "L1_Peripheral/pit.hpp"
#include "L1_Peripheral/uart.hpp"
#include "L1_Peripheral/post.hpp"
#include "L3_Application/ring_buf.hpp"
#include "L3_Application/cli.hpp"
#include "comms.hpp"

#define INTCON_LV7_BRK          0x80
#define INTCON_LV6_SYSTICK      0x40
#define INTCON_LV5_USTIMER      0x20
#define INTCON_LV4_MSTIMER      0x10
#define INTCON_LV3_UART         0x08

extern volatile uint8_t __systick_flag;
extern volatile uint8_t __interval_1ms_flag;
extern volatile uint8_t __interval_1us_flag;
extern volatile uint32_t __systick_count;

extern cpu_68000 cpu;
extern SystemController system_controller;
extern Post post;
extern ProgrammableIntervalTimer pit;
extern InterruptController interrupt_controller;
extern Uart uart;

/* Function prototypes */
void sbc_initialize(void);
void set_post(uint8_t value);
bool get_break(void);

#endif /* _SBC_H_ */
