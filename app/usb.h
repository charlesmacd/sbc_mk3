
#ifndef _USB_H_
#define _USB_H_

#include "sys_types.h"

/* USB (UM245R) flags */
#define F_USB_RXE			0x80
#define F_USB_TXF			0x40
#define F_USB_PWE			0x20


void usb_sendb(uint8_t data);
uint8_t usb_getb(void);
void usb_send(uint8_t *data, uint32_t size);
void usb_get(uint8_t *data, uint32_t size);
uint32_t usb_getl(void);
bool usb_is_data_ready(void);
bool usb_is_enumerated(void);

#endif /* _USB_H_ */
