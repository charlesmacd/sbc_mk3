
#-------------------------------------------------------------------------------
# uart_asm.s
#-------------------------------------------------------------------------------

                .include "macros.inc"
                .include "hw_defs.inc"

UART_BAUD_9600          =       0xBB
UART_BAUD_19200         =       0x33
UART_BAUD_28800         =       0x44
UART_BAUD_38400         =       0xCC
UART_BAUD_57600         =       0x55
UART_BAUD_115200        =       0x66


                .extern uart_tx_ringbuf
                .extern uart_rx_ringbuf
                .extern uart_tx_size


#-------------------------------------------------------------------------------
# Send character to SCC2691 UART
#-------------------------------------------------------------------------------
                .global uart_putch_polling
uart_putch_polling:
                move.b  d0, (REG_THR).w
        1:      btst    #B_STATUS_TXRDY, (REG_SR).w
                beq.s   1b
                rts

#-------------------------------------------------------------------------------
# Put string
#-------------------------------------------------------------------------------
                .globl uart_puts_polling
uart_puts_polling:
                movem.l d0/a0, -(sp)
        2:      move.b  (a0)+, d0
                tst.b   d0
                beq.s   1f
                jsr     uart_putch_polling
                bra.s   2b
        1:      movem.l (sp)+, d0/a0
                rts



#-------------------------------------------------------------------------------
# Reset UART
#-------------------------------------------------------------------------------
                .global uart_reset
uart_reset:
                move.b  #1, (REG_UART_RESET).w
                jsr     uart_delay
                move.b  #0, (REG_UART_RESET).w
                jsr     uart_delay

                rts

#-------------------------------------------------------------------------------
# Initialize UART
#-------------------------------------------------------------------------------
                .global uart_initialize
uart_initialize:
                # Low commands
                move.b  #CR_DISABLE_TRANSMITTER, (REG_CR).w
                jsr     uart_delay
                move.b  #CR_DISABLE_RECEIVER, (REG_CR).w
                jsr     uart_delay

                # High commands
                move.b  #CR_RESET_MR, (REG_CR).w
                jsr     uart_delay
                move.b  #CR_RESET_RECEIVER, (REG_CR).w
                jsr     uart_delay
                move.b  #CR_RESET_TRANSMITTER, (REG_CR).w
                jsr     uart_delay
                move.b  #CR_RESET_ERROR_STATUS, (REG_CR).w
                jsr     uart_delay
                move.b  #CR_RESET_BREAK_CHANGE_INT, (REG_CR).w
                jsr     uart_delay
                move.b  #CR_STOP_CT, (REG_CR).w
                jsr     uart_delay
                move.b  #CR_RESET_MPI_CHANGE_INT, (REG_CR).w
                jsr     uart_delay
                move.b  #CR_RESET_MR, (REG_CR).w
                jsr     uart_delay

                # Enable RxRDY/FFUL interrupt (bit 2)
                move.b  #0x04, (REG_IMR).w

                # Set MR1 to 8-N-1
                move.b  #0b00010011, (REG_MR).w

                # Set MR2 to 1 stop bit
                move.b  #0b00000111, (REG_MR).w

                # Toggle baud rate test mode flip-flop from 0 (reset) to 1
                tst.b   (REG_BRG_TEST).w

                # Set baud rate to 1200 -> 115,200
                # NOTE: See Table 6 (Baud Rates Extended) in SCC2691 data sheet page 20
                move.b  #UART_BAUD_115200, (REG_CSR).w

                # Program auxilliary control register
                # Set PWRDN=off (ACR[3] = 0b)
                # Set TxC (1x) as MPO (ACR[2:0] = 011b)
                move.b  #0b00001011, (REG_ACR).w

                # Start TX and RX 
                move.b  #0x05, (REG_CR).w

                rts                


#-------------------------------------------------------------------------------
# Short delay
#-------------------------------------------------------------------------------
uart_delay:
                move.l  d7, -(sp)
                moveq   #0x7F, d7
        1:      nop                
                dbra    d7, 1b
                move.l  (sp)+, d7
                rts

#-------------------------------------------------------------------------------
# UART interrupt handler
# Called for each byte transmitted or received
#-------------------------------------------------------------------------------
                .global uart_isr
uart_isr:
                # Save registers
                movem.l d0-d7/a0-a6, -(sp)

                # Check for RXRDY interrupt pending
                btst    #B_UART_ISR_RXRDY, (REG_ISR).w
                beq.s   .check_tx_cause

                # RXRDY interrupt was pending
                # This interrupt means RxRDY or FFULL condition
                # Process receiving a character
        .rx_next_char:

                # Get byte from UART FIFO                
                move.b  (REG_RHR).w, d0

                # Insert byte to ring buffer
                move.l  d0, -(sp)
                pea     uart_rx_ringbuf
                jsr     ringbuf_insert
                addq.l  #8, sp

                # Check if more data is remaining
                btst    #B_UART_SR_RXRDY, (REG_SR).w
                bne.s   .rx_next_char

                # EDIT fall through so we can handle simulataneous conditions
                #  bra.s   .exit

                #----------------------------------

                # Check for transmit interrupt
        .check_tx_cause:

                # TxRDY : Set when THR is empty and can be written into (after 8 bits shifted out)
                #         Clr when CPU writes into THR

                # TxEMT : Set on transmitter underrun (TXEMT=1, TXRDY=1)
                #       : Clr when writing into THR,
                #         Clr when pending transmitter disable executed
                #         Clr when transmitter disabled in underrun

                btst    #B_UART_ISR_TXRDY, (REG_ISR).w
                beq.s   .exit

                # Get TX ring buffer size
                pea     uart_tx_ringbuf
                jsr     ringbuf_size
                addq.l  #4, sp
                tst.b   d0
                beq.s   .tx_ringbuf_empty

                # Remove character from ring buffer
                pea     uart_tx_ringbuf
                jsr     ringbuf_remove
                addq.l  #4, sp

                # Send character to UART
                move.b  d0, (REG_THR).w
                bra.s   .exit

        .tx_ringbuf_empty:        
                # Write 0x04 to IMR
                # IMR[2] (RxRDY interrupt) = 1
                # IMR[1] (TxEMT interrupt) = 0
                # IMR[0] (TxRDY interrupt) = 0
                # Turn off transmitter empty interrupt if there's nothing to send
                move.b  #F_UART_IMR_RXRDY, (REG_IMR).w
        .exit:
                # Restore registers
                movem.l (sp)+, d0-d7/a0-a6

                # Acknowledge interrupt
                # (Not actually used, INTC is on D7-D4 only)
                move.b  #F_UART_IRQ_ACK, (REG_IPEND_CLR).w

                # Return from exception
                rte                

#-------------------------------------------------------------------------------
#
#-------------------------------------------------------------------------------
