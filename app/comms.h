
#ifndef _COMMS_H_
#define _COMMS_H_

#include "sbc.h"
#include "sys_types.h"
#include "usb.h"
#include "main.h"
#include "uart.h"

/* ASCII codes */
#define ASCII_ESC			0x1B

void cmd_echo(void);
void cmd_download(void);
void check_comms_dispatch(void);


#endif /* _COMMS_H_ */
