
#ifndef _USB_H_
#define _USB_H_

#include "../sys_types.hpp"
#include "system_controller.hpp"

/* USB (UM245R) flags */
#define F_USB_RXE			0x80
#define F_USB_TXF			0x40
#define F_USB_PWE			0x20


struct usb_register_tag
{
	union
	{
		struct
		{
            __OM uint16_t DATA;
        } w;

		struct
		{
            __IM uint16_t DATA;
		} r;	
	} reg;
} __attribute__((packed, aligned(2)));

typedef usb_register_tag usb_register_t;

class FtdiUSB
{
public:
    usb_register_t *device;
    SystemController *sysc;

    void initialize(SystemController *sctrl, usb_register_t *external_reg = NULL)
    {
        device = external_reg;
        if(device == NULL)
        {
            device = (usb_register_t *)0xFFFFA000;
        }

        sysc = sctrl;
    }

    void write(uint8_t data)
    {
        device->reg.w.DATA = data;
    }

    uint8_t read(void)
    {
        return device->reg.r.DATA;
    }

    void write(uint8_t *data, uint32_t size)
    {        
        while(size--)
        {            
            device->reg.w.DATA = *data++;
        }
    }

    void read(uint8_t *data, uint32_t size)
    {
        while(size--)
        {
            *data++ = device->reg.r.DATA;
        }
    }

    bool is_rx_empty(void)
    {
        return (sysc->get_status() & SYSCON_STATUS_USB_RXF) ? true : false;
    }

    bool is_enumerated(void)
    {
        return (sysc->get_status() & SYSCON_STATUS_USB_PWE) ? true : false;
    }
};



void usb_sendb(uint8_t data);
uint8_t usb_getb(void);
void usb_send(const uint8_t *data, uint32_t size);
void usb_get(uint8_t *data, uint32_t size);
uint32_t usb_getl(void);
void usb_sendl(uint32_t value);
bool usb_is_data_ready(void);
bool usb_is_enumerated(void);
void usb_send_pstr(const uint8_t *msg);

#endif /* _USB_H_ */
