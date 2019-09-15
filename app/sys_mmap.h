
#ifndef _SYS_MMAP_H_
#define _SYS_MMAP_H_

/* Hardware register locations*/
#define PIO_BASE			0xFFFF8001
#define POST_BASE			(PIO_BASE + 0x0000)
#define UART_BASE			(PIO_BASE + 0x1000)
#define PIT_BASE			(PIO_BASE + 0x1040)
#define I2C_BASE			(PIO_BASE + 0x1080)
#define I2C_IACK			(PIO_BASE + 0x10C0)
#define USB_BASE			(PIO_BASE + 0x2000)

#endif /* _SYS_MMAP_H_ */
