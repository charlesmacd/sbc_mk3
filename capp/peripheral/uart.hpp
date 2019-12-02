
#ifndef _UART_H_
#define _UART_H_

#include "../sys_types.hpp"
#include "../sbc.hpp"
#include "../utility/ringbuf.h"

/* UART (SCC2691) flags */
#define F_UART_TXRDY		0x04
#define F_UART_RXRDY		0x01
#define F_STATUS_BREAK		0x01
#define uart_putch          uart_sendb
#define uart_getch          uart_getb

extern ringbuf_t uart_rx_ringbuf;
extern ringbuf_t uart_tx_ringbuf;

uint8_t uart_tx_size(void);
uint8_t uart_rx_count(void);
uint8_t uart_rx_read(void);
void uart_tx_write(uint8_t data);
bool uart_keypressed(void);
char uart_readkey();
void uart_sendb(uint8_t data);
uint8_t uart_getb(void);
void uart_send(const uint8_t *data, uint32_t size);
void uart_get(uint8_t *data, uint32_t size);
void uart_puts(const char *msg);
void uart_hexout(uint8_t digit);
void uart_printhexb(uint8_t value);
void uart_printhexw(uint16_t value);
void uart_printhexl(uint32_t value);
void uart_printd(uint32_t value);
void uart_printd_padding(uint32_t value, uint8_t padding_width, uint8_t padding_type);


#endif /* _UART_H_ */
