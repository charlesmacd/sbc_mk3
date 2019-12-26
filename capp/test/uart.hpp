
#ifndef _UART_H_
#define _UART_H_


/*


MR1:
MR[4:3] = {Has parity, Force parity, No parity, Special mode}
MR[2]   = Parity is even or odd
MR[1:0] = {5, 6, 7, 8} bits per character

MR2:
MR[3:0] = Stop bit length (0.5, 1.0, 1.5, 2.0)



*/

#include "../sys_types.hpp"
#include "../L3_Application/ring_buf.hpp"

typedef struct
{
    volatile uint8_t *REG_CR;
    volatile uint8_t *REG_MR;
    volatile uint8_t *REG_SR;
    volatile uint8_t *REG_ISR;
    volatile uint8_t *REG_RHR;
    volatile uint8_t *REG_THR;
    volatile uint8_t *REG_IMR;
    volatile uint8_t *REG_CSR;
    volatile uint8_t *REG_ACR;
    volatile uint8_t *REG_BRG_TEST;
} uart_register_t;


class Uart
{
private:
    void command_delay(void);
    void send_command(uint8_t data);
    void reset(void);
public:

    uart_register_t reg;
    RingBuffer rx_ringbuf;
    RingBuffer tx_ringbuf;
    uint8_t state_imr;

    void enable_interrupts(uint8_t mask);
    void disable_interrupts(uint8_t mask);

    void initialize(void);

    /* Write one byte */
    void write(uint8_t value);

    /* Write multiple bytes */
    void write(uint8_t *data, uint32_t size);

    /* Read one byte, non-blocking */
    uint8_t read(void);

    /* Read one byte, blocking */
    uint8_t read_blocking(void);

    /* Read multiple bytes */
    void read(uint8_t *data, uint32_t size);

    /* Configure UART */
    void set_baud_rate(int rate);

    /* Write string */
    void puts(const char *str);

    /* Wait for data */
    bool keypressed(void);
};


#endif /* _UART_H_ */