/*
	File:
		usb.cpp
	Author:
		Charles MacDonald
	Notes:
		None
*/

#include <stdint.h>
#include "../sbc.hpp"

/*
	CPLD implements auto-wait feature where DTACK# is delayed during the following conditions:
	- RXE# negated during read cycle to USB data port.
	- TXF# negated during write cycle to USB data port.
	Polling is no longer needed but available if changes the CPLD are made.
*/
#define USB_USE_POLLING		1

static volatile uint8_t *USB_DATA		= (volatile uint8_t *)(USB_BASE);

static volatile uint8_t *REG_STATUS 	= (volatile uint8_t *)(PIO_BASE  + 0x40);


/* Send byte to host */
void usb_sendb(uint8_t data)
{
#if USB_USE_POLLING
	while(REG_STATUS[0] & F_USB_TXF)
	{
		;
	}
#endif
	USB_DATA[0] = data;
}



/* Get byte from host */
uint8_t usb_getb(void)
{
#if USB_USE_POLLING
	while(REG_STATUS[0] & F_USB_RXE)
	{
		;
	}
#endif	
	return USB_DATA[0];
}



/* Send byte array to the host */
void usb_send(uint8_t *data, uint32_t size)
{
	while(size--)
	{
		USB_DATA[0] = *data++;
	}
}



/* Get byte array from the host */
void usb_get(uint8_t *data, uint32_t size)
{
	while(size--)
	{
		*data++ == USB_DATA[0];
	}
}



/* Get 32-bit word from host */
uint32_t usb_getl(void)
{
	uint32_t temp;
	temp  = usb_getb() << 24;
	temp |= usb_getb() << 16;
	temp |= usb_getb() << 8;
	temp |= usb_getb() << 0;
	return temp;
}



/* Check if there's any data in the RX FIFO */
bool usb_is_data_ready(void)
{
	return (REG_STATUS[0] & F_USB_RXE) ? false : true;
}



/* Check if the device has enumerated */
bool usb_is_enumerated(void)
{
	return (REG_STATUS[0] & F_USB_PWE) ? false : true;
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
