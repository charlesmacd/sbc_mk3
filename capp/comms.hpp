
#ifndef _COMMS_H_
#define _COMMS_H_

#include "sbc.hpp"
#include "sys_types.hpp"
#include "L1_Peripheral/usb.hpp"
#include "L1_Peripheral/uart.hpp"
#include "L3_Application/mutex.hpp"
#include "main.hpp"

/* ASCII codes */
#define ASCII_ESC			0x1B

void cmd_echo(void);
void cmd_download(void);
void check_comms_dispatch(void);
void pc_exit(void);

void usb_send_pstr(const uint8_t *msg);
int pc_fopen(const char *filename, const char *mode);
void pc_fclose(uint8_t handle);
void pc_puts(const char *msg);
int pc_fwrite(uint8_t handle, const uint8_t *buf, uint32_t size);

extern Mutex comms_mutex;


#endif /* _COMMS_H_ */
