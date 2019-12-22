
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


                .extern ringbuffer_tx_insert
                .extern ringbuffer_rx_remove
                .extern ringbuffer_rx_size

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
#
#-------------------------------------------------------------------------------


                .globl  debug_puts
debug_puts:
                move.l  4(sp), a0
        2:
                move.b  (a0)+, d0
                tst.b   d0
                beq.s   1f
                jsr     uart_putch_polling
                bra.s   2b
        1: 
                rts

                .globl debug_putch
debug_putch:
                move.l  4(sp), d0
                jsr     uart_putch_polling
                rts                

                .globl debug_printhexl

debug_printhexl:
                move.l  4(sp), d1
                lea     hextable(pc), a1
                .rept   8
                rol.l   #4, d1
                moveq   #0x0F, d0
                and.b   d1, d0
                move.b  (a1, d0.w), d0
                jsr     uart_putch_polling
                .endr 
                
                rts

                .align  2
hextable:       
                .asciz    "0123456789ABCDEF"
                .align  2
