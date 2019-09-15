#ifndef _USB_H_
#define _USB_H_

#include <stdint.h>
#include "FTD2XX.H"

/* Function prototypes */
void usb_logerror(int level, char *format, ...);
char *usb_getstatus(FT_STATUS status);
void usb_init(bool list_devices = false);
void usb_shutdown(void);
void usb_send(uint8_t *buffer, uint32_t size);
void usb_sendb(uint8_t value);
int usb_queue(void);
void usb_sendw(uint16_t value);
void usb_sendl(uint32_t value);
void usb_get(uint8_t *buffer, uint32_t size);
uint8_t usb_getb(void);
uint16_t usb_getw(void);
uint32_t usb_getl(void);
void usb_purge(void);
void usb_reset(void);

#endif /* _USB_H_ */
