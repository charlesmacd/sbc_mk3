/*
	File:
		uart.h
	Notes:
		None
*/

#include <cstdint>
#include <cstring>
#include "uart.hpp"
#include "../L3_Application/ring_buf.hpp"

//# CR[7:4] values: 
#define CR_NOP                                 0x00
#define CR_RESET_MR                            0x10
#define CR_RESET_RECEIVER                      0x20
#define CR_RESET_TRANSMITTER                   0x30
#define CR_RESET_ERROR_STATUS                  0x40
#define CR_RESET_BREAK_CHANGE_INT              0x50
#define CR_START_BREAK                         0x60
#define CR_STOP_BREAK                          0x70
#define CR_START_CT                            0x80
#define CR_STOP_CT                             0x90
#define CR_ASSERT_RTS                          0xA0
#define CR_NEGATE_RTS                          0xB0
#define CR_RESET_MPI_CHANGE_INT                0xB0
#define CR_RESERVED_0                          0xC0
#define CR_RESERVED_1                          0xD0 /* Undocumented */
#define CR_RESERVED_2                          0xE0
#define CR_RESERVED_3                          0xF0

//#define # CR[3:0] values:
#define CR_DISABLE_TRANSMITTER                 0x08
#define CR_ENABLE_TRANSMITTER                  0x04
#define CR_DISABLE_RECEIVER                    0x02
#define CR_ENABLE_RECEIVER                     0x01

#define UART_BAUD_9600          		       0xBB
#define UART_BAUD_19200         		       0x33
#define UART_BAUD_28800         		       0x44
#define UART_BAUD_38400         		       0xCC
#define UART_BAUD_57600         		       0x55
#define UART_BAUD_115200        		       0x66

extern Uart uart;
RingBuffer uart_rx_ringbuf;
RingBuffer uart_tx_ringbuf;


/* Short delay for command processing */
void Uart::command_delay(void)
{
	for(int i = 0; i < 0x80; i++)
	{
		__asm__ __volatile__ ("nop");
	}
}

/* Send command and delay */
void Uart::send_command(uint8_t value)
{
	*reg.REG_CR = value;
	command_delay();
}

void Uart::set_baud_rate(int rate)
{
	/* Reset UART via RESET line */
	system_controller.assert_peripheral_reset(PERIPHERAL_UART, true);
	command_delay();
	system_controller.assert_peripheral_reset(PERIPHERAL_UART, false);
	command_delay();

	/* Send low commands */
	send_command(CR_DISABLE_TRANSMITTER);
	send_command(CR_DISABLE_RECEIVER);

	/* Send high commands */
	send_command(CR_RESET_MR);
	send_command(CR_RESET_RECEIVER);
	send_command(CR_RESET_TRANSMITTER);
	send_command(CR_RESET_ERROR_STATUS);
	send_command(CR_RESET_BREAK_CHANGE_INT);
	send_command(CR_STOP_CT);
	send_command(CR_RESET_MPI_CHANGE_INT);
	send_command(CR_RESET_MR);

	/* Enable RxRDY/FFUL interrupt (bit 2) */
	*reg.REG_IMR = 0x04;

	/* Set MR1 to 8-N-1 */
	*reg.REG_MR = 0x013;

	/* Set MR2 to 1 stop bit */
	*reg.REG_MR = 0x07;

	/* Toggle baud rate test mode flip-flop from 0 (reset) to 1 */
	uint8_t temp;
	temp = *reg.REG_BRG_TEST;

	// Set baud rate to 1200 -> 115,200
	// # NOTE: See Table 6 (Baud Rates Extended) in SCC2691 data sheet page 20
	switch(rate)
	{
		case 9600:
			*reg.REG_CSR = UART_BAUD_9600;
			break;
		case 19200:
			*reg.REG_CSR = UART_BAUD_19200;
			break;
		case 28800:
			*reg.REG_CSR = UART_BAUD_28800;
			break;
		case 38400:
			*reg.REG_CSR = UART_BAUD_38400;
			break;
		case 57600:
			*reg.REG_CSR = UART_BAUD_57600;
			break;
		case 115200:
			*reg.REG_CSR = UART_BAUD_115200;
			break;
	}

  	// # Program auxilliary control register
    // # Set PWRDN=off (ACR[3] = 0b)
    // # Set TxC (1x) as MPO (ACR[2:0] = 011b)
  	*reg.REG_ACR = 0x0B;

	// # Start TX and RX 
	*reg.REG_CR = 0x05;

	/* Initialize ring buffers */
	uart_tx_ringbuf.initialize(0x80);
	uart_rx_ringbuf.initialize(0x80);
}



/* Initialize UART */
void Uart::initialize(void)
{
	reg.REG_MR         = (volatile uint8_t *)(UART_BASE + 0x00);
	reg.REG_SR         = (volatile uint8_t *)(UART_BASE + 0x02);
	reg.REG_CSR        = (volatile uint8_t *)(UART_BASE + 0x02);
	reg.REG_BRG_TEST   = (volatile uint8_t *)(UART_BASE + 0x04);
	reg.REG_CR         = (volatile uint8_t *)(UART_BASE + 0x04);
	reg.REG_RHR        = (volatile uint8_t *)(UART_BASE + 0x06);
	reg.REG_THR        = (volatile uint8_t *)(UART_BASE + 0x06);
	reg.REG_ACR        = (volatile uint8_t *)(UART_BASE + 0x08);
	reg.REG_IMR        = (volatile uint8_t *)(UART_BASE + 0x0A);
	reg.REG_ISR        = (volatile uint8_t *)(UART_BASE + 0x0A);

	set_baud_rate(115200);
}

void Uart::write(uint8_t data)
{
	uart_tx_ringbuf.insert(data);
	/* Rewrite IMR to enable transceiver */
	*reg.REG_IMR = 0x05;
}

void Uart::write(uint8_t *data, uint32_t size)
{
	for(int i = 0; i < size; i++)
	{
		write(data[i]);
	}
}

uint8_t Uart::read(void)
{
	return uart_rx_ringbuf.remove();
}

void Uart::read(uint8_t *data, uint32_t size)
{
	for(int i = 0; i < size; i++)
	{
		data[i] = read();
	}
}

void Uart::puts(const char *str)
{
	write((uint8_t *)str, strlen(str));
}

bool Uart::keypressed(void)
{
	return (uart_rx_ringbuf.size() == 0) ? false : true;
}

uint8_t Uart::read_blocking(void)
{
	while(!keypressed())
	{
		;
	}

	return read();
}


/* End */















void uart_hexout(uint8_t digit)
{
	uart.write("0123456789ABCDEF"[digit & 0x0F]);
}

void uart_printhexb(uint8_t value)
{
	uart_hexout((value >> 4) & 0x0F);
	uart_hexout((value >> 0) & 0x0F);
}

void uart_printhexw(uint16_t value)
{
	uart_printhexb(	(value >> 8) & 0xFF);
	uart_printhexb((value >> 0) & 0xFF);
}

void uart_printhexl(uint32_t value)
{
	uart_printhexw((value >> 16) & 0xFFFF);
	uart_printhexw((value >> 0 ) & 0xFFFF);
}


/* Print 32-bit value as decimal with zero padding */
void uart_printd_padding(uint32_t value, uint8_t padding_width, uint8_t padding_type)
{
	uint8_t stack[10];
	uint8_t index = 0; 

	/* Push decimal digits back-to-front */
	do {
		stack[index++] = (value % 10);
		value /= 10;		
	} while(value);

	
	/* Print leading zeros */
	if(padding_width)
	{
		for(int k = index; k < padding_width; k++)
		{
			if(padding_type == 0)
				uart_hexout(0);
			else
			{
				uart.write(' ');
			}
		}
	}

	/* Pop digits and print them in order */
	while(index--)
	{
		uart_hexout(stack[index]);
	}
}


/* Print 32-bit value as decimal */
void uart_printd(uint32_t value)
{
	uint8_t stack[10];
	uint8_t index = 0; 

	/* Push decimal digits back-to-front */
	do {
		stack[index++] = (value % 10);
		value /= 10;		
	} while(value);

	/* Pop digits and print them in order */
	while(index--)
	{
		uart_hexout(stack[index]);
	}
}

/* End */