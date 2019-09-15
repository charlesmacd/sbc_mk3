
#ifndef _SBC_H_
#define _SBC_H_

#include <stdint.h>


#include "crt0.h"
#include "printf.h"
#include "sys_mmap.h"
#include "main.h"
#include "usb.h"
#include "uart.h"
#include "comms.h"
//#include "flash.h"
#include "ringbuf.h"
#include "cli.h"


extern volatile uint8_t *USB_DATA;
extern volatile uint8_t *REG_I2C;
extern volatile uint8_t *REG_POST;
extern volatile uint8_t *REG_STATUS;
extern volatile uint8_t *REG_TIMER_1US_ENABLE;
extern volatile uint8_t *REG_TIMER_1MS_ENABLE;
extern volatile uint16_t *REG_USER_LED;
extern volatile uint16_t *REG_IPENDING;
extern volatile uint16_t *REG_IPEND_CLR;
extern volatile uint16_t *REG_IPEND_SET;
extern volatile uint16_t *REG_IENABLE;
extern volatile uint16_t *REG_IMASKED;
extern volatile uint16_t *REG_IPRIORITY;
extern volatile uint16_t *REG_STATUS0;
extern volatile uint16_t *REG_STATUS1;
extern volatile uint16_t *REG_STATUS2;
extern volatile uint16_t *REG_STATUS3;
extern volatile uint8_t *REG_SR;
extern volatile uint8_t *REG_ISR;
extern volatile uint8_t *REG_RHR;
extern volatile uint8_t *REG_IMR;

#define F_ISR_RXRDY			0x04
#define F_SR_RXRDY			0x01
#define WORD_ADDRESS(x)		(x&~1)

extern volatile uint16_t *REG_PIT;

extern uint8_t __systick_flag;
extern uint8_t __interval_1ms_flag;
extern uint8_t __interval_1us_flag;

extern ringbuf_t uart_rx_ringbuf;
extern ringbuf_t uart_tx_ringbuf;

/* Function prototypes */
void set_post(uint8_t value);
bool get_break(void);
void delay_ms(uint16_t duration);

uint8_t uart_tx_size(void);
uint8_t uart_rx_count(void);
uint8_t uart_rx_read(void);
void uart_tx_write(uint8_t data);
bool uart_keypressed(void);
char uart_readkey();

#endif /* _SBC_H_ */
