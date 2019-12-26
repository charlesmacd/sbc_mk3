/*
	File:
		uart.h
	Notes:
		None
*/

#include <cstdint>
#include <cstring>
#include "../L1_Peripheral/system_controller.hpp"
#include "../L3_Application/ring_buf.hpp"
#include "../sbc.hpp"
#include "uart.hpp"

#define F_ISR_RXRDY			0x04
#define F_SR_RXRDY			0x01

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

#define UART_INTEN_TXRDY						0x01
#define UART_INTEN_RXRDY						0x04


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

void Uart::reset(void)
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

	/* Set MR1 to 8-N-1 */
	*reg.REG_MR = 0x013;

	/* Set MR2 to 1 stop bit */
	*reg.REG_MR = 0x07;

	/* Toggle baud rate test mode flip-flop from 0 (reset) to 1 */
	(void)*reg.REG_BRG_TEST;
}

void Uart::set_baud_rate(int rate)
{
	reset();

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
	tx_ringbuf.initialize(0x80);
	rx_ringbuf.initialize(0x80);

	/* Enable RxRDY/FFUL interrupt (bit 2) */
	enable_interrupts(UART_INTEN_RXRDY);
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

void Uart::enable_interrupts(uint8_t mask)
{
	state_imr |= mask;
	*reg.REG_IMR = state_imr;
}

void Uart::disable_interrupts(uint8_t mask)
{
	state_imr &= ~mask;
	*reg.REG_IMR = state_imr;
}

void Uart::write(uint8_t data)
{
	if(tx_ringbuf.full())
	{
		enable_interrupts(UART_INTEN_TXRDY);
		while(tx_ringbuf.full())
		{
			;
		}			
	}

	tx_ringbuf.insert(data);
	enable_interrupts(UART_INTEN_TXRDY);
}

void Uart::write(const uint8_t *data, uint32_t size)
{
	for(size_t i = 0; i < size; i++)
	{
		if(tx_ringbuf.full())
		{
			enable_interrupts(UART_INTEN_TXRDY);
			while(tx_ringbuf.full())
			{
				;
			}			
		}

		tx_ringbuf.insert(data[i]);
	}
	enable_interrupts(UART_INTEN_TXRDY);
}

uint8_t Uart::read(void)
{
	return rx_ringbuf.remove();
}

void Uart::read(uint8_t *data, uint32_t size)
{
	for(size_t i = 0; i < size; i++)
	{
		data[i] = rx_ringbuf.remove();
	}
}


bool Uart::keypressed(void)
{
	return rx_ringbuf.empty() ? false : true;
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









/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#define B_UART_ISR_RXRDY  				2
#define B_UART_SR_RXRDY   				0
#define B_UART_ISR_TXRDY  				0

extern "C" {

//
// __attribute__((interrupt))
//

void UartSCC2681_ISR(void *param)
{
//	const uint8_t F_ISR_RXRDY = (1 << B_UART_ISR_RXRDY);
//	const uint8_t F_SR_RXRDY  = (1 << B_UART_SR_RXRDY);
	const uint8_t F_ISR_TXRDY = (1 << B_UART_ISR_TXRDY);

	/* TODO: Point to current device context (ISR preamble should assign this) */
	Uart *device = (Uart *)param;

	/* If RXRDY cause was set, empty read FIFO into RX ring buffer */
	if(*device->reg.REG_ISR & F_ISR_RXRDY)
	{
		do {			
			if(device->rx_ringbuf.full())
			{
				/* Can't accept more data; turn off receiver interrupt */
//				device->disable_interrupts(UART_INTEN_RXRDY);
// we need ringbuf remove to notify to turn back on
			}
			else
			{
				char ch = *device->reg.REG_RHR;
				device->rx_ringbuf.insert(ch); /* Note: If full will discard */
			}
		} while(device->reg.REG_SR[0] & F_SR_RXRDY);
	}

	/* If TXRDY cause was set, output a character if present to transmitter */
	if(*device->reg.REG_ISR & F_ISR_TXRDY)
	{
		if(!device->tx_ringbuf.empty())
		{
			char ch;
			ch = device->tx_ringbuf.remove();
			*device->reg.REG_THR = ch;
		}
		else
		{
			/* Nothing to send; disable transmitter interrupt */
			device->disable_interrupts(UART_INTEN_TXRDY);
		}
	}

	/* Acknowledge interrupt (not actually needed) */
	interrupt_controller.clear_pending(0x08);

} /* handler_isr */

} /* extern "C" */



/* End */






