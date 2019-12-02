
#ifndef _COMMS_H_
#define _COMMS_H_

#include "sbc.hpp"
#include "sys_types.hpp"
#include "peripheral/usb.hpp"
#include "main.hpp"
#include "peripheral/uart.hpp"

/* ASCII codes */
#define ASCII_ESC			0x1B

void cmd_echo(void);
void cmd_download(void);
void check_comms_dispatch(void);


#endif /* _COMMS_H_ */
