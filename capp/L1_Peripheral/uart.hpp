
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

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

struct uart_register_tag
{
	union
	{
		struct
		{
            union 
            {
                __OM uint16_t MR1;
                __OM uint16_t MR2;
            };
            __OM uint16_t CSR;
            __OM uint16_t CR;
            __OM uint16_t THR;
            __OM uint16_t ACR;
            __OM uint16_t IMR;
            __OM uint16_t CTUR;
            __OM uint16_t CTLR;
        } w;

		struct
		{
            union 
            {
                __IM uint16_t MR1;
                __IM uint16_t MR2;
            };
            __IM uint16_t SR;
            __IM uint16_t BRG_TEST;
            __IM uint16_t RHR;
            __IM uint16_t TEST;
            __IM uint16_t ISR;
            __IM uint16_t CTU;
            __IM uint16_t CTL;
		} r;	
	} reg;
} __attribute__((packed, aligned(2)));

typedef uart_register_tag uart_register_t;

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/


class UartDevice
{
public:
    virtual void write(uint8_t value) = 0;
    virtual void write(const uint8_t *data, uint32_t size) = 0;
    virtual uint8_t read(void) = 0;
    virtual uint8_t read_blocking(void) = 0;
    virtual void read(uint8_t *data, uint32_t size) = 0;

};

class Uart : public UartDevice 
{
private:

    void write_mr1(uint8_t value);
    void command_delay(void);
    void send_command(uint8_t data);
    void reset(void);
public:

    uart_register_t *dev;
    
    RingBuffer rx_ringbuf;
    RingBuffer tx_ringbuf;
    uint8_t state_imr;

    Uart()
    {
        
    }

    void enable_interrupts(uint8_t mask);
    void disable_interrupts(uint8_t mask);

    void initialize(uart_register_t *external_reg = NULL);

    /* Write one byte */
    void write(uint8_t value);

    /* Write multiple bytes */
    void write(const uint8_t *data, uint32_t size);

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
