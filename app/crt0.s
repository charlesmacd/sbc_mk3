#------------------------------------------------------------------------------
# crt0.s
#
#------------------------------------------------------------------------------

# Memory map for RAM relocation
# 100000-107FFF : Start-up code in boot ROM (32K)
# 108000-10BFFF : Vector redirect (16K)
# 10C000-10FFFF : Stack and heap (16K)
# 110000-1FFFFF : Free memory (stack, heap, etc.)

#------------------------------------------------------------------------------
# Include files
#------------------------------------------------------------------------------

                .include "macros.inc"
                .include "hw_defs.inc"

# Just RAM things
WRAM_RESERVED           =       0x10000
LOAD_ADDRESS            =       (WRAM_BASE + WRAM_RESERVED)
LOCAL_STACK_TOP         =       (WRAM_BASE + WRAM_RESERVED)

OPCODE_JMP_ABS32        =       0x4EF9   
VECTOR_TABLE_BASE       =       0x0000             

#------------------------------------------------------------------------------
# Entry point
#------------------------------------------------------------------------------

reset:
                # Set up stacks
                move.w  #0x2700, sr              
                lea     LOCAL_STACK_TOP, a7
                move.l  a7, usp                

                # Reset UART
                jsr     uart_reset
                jsr     uart_initialize

                # Print startup message
                lea     ansi_clrscr, a0
                jsr     uart_puts
                lea     msg_startup, a0
                jsr     uart_puts


                # Patch autovectors to point to our ISRs
                moveq   #(7)-1, d0
                lea     __autovector_isr_list(pc), a0
                lea     WRAM_BASE+(AUTOVECTOR_ISR_NUMBER*8), a1
    isr_setup:  move.w  #OPCODE_JMP_ABS32, (a1)+
                move.l  (a0)+, (a1)+
                lea     2(a1), a1
                dbra    d0, isr_setup

                .if     1
                lea     WRAM_BASE+(25*8)+2, a0
                move.l  #__level1_isr, (a0)
                lea     WRAM_BASE+(26*8)+2, a0
                move.l  #__level2_isr, (a0)
                lea     WRAM_BASE+(27*8)+2, a0
                move.l  #__level3_isr, (a0)
                lea     WRAM_BASE+(28*8)+2, a0
                move.l  #__level4_isr, (a0)
                lea     WRAM_BASE+(29*8)+2, a0
                move.l  #__level5_isr, (a0)
                lea     WRAM_BASE+(30*8)+2, a0
                move.l  #__level6_isr, (a0)
                lea     WRAM_BASE+(31*8)+2, a0
                move.l  #__level7_isr, (a0)
                .endif

                # Run main function 
                jsr     main

                lea     msg_restart, a0
                jsr     uart_puts

                lea     (VECTOR_TABLE_BASE).w, a0
                move.l  (a0)+, sp
                move.l  sp, usp
                move.l  (a0)+, a0
                jmp     (a0)

__autovector_isr_list:
                dc.l    __level1_isr                
                dc.l    __level2_isr                
                dc.l    __level3_isr                
                dc.l    __level4_isr                
                dc.l    __level5_isr                
                dc.l    __level6_isr                
                dc.l    __level7_isr


# negedge of SYSTICK happens
# ~220ns later IPEND is set
# edge detector takes 2 cycles (125ns x 2 = 250ns for data to pass through)
# this is around the time VPA is set up
# 3020ns later VPA cycle happens (~24 cycles at 8 mhz)
# vpa pulse width is 1560ns (12 cycles)
# IPEND clear 6550ns later (52 cycles)
#
# jmp takes 12 cycles (3/0)             rdop,rd adl, rd adh
# move.w #imm16,(xxx).w takes 16 cycles (3/1)
#
# 72 cycles total
#
# seems ok then?
#
# see vpa cycles every 55,750us
# ok that's because timer call to 1ms/1us were on
# need to change thier mode to one-shot
# normal vpa amount now

                .extern uart_handler
                .extern interval_1ms_handler
                .extern interval_1us_handler

#--------------------------------------------------------
#
#--------------------------------------------------------
__level1_isr:
                move.b  #0x02, (REG_IPEND_CLR).w
                rte                
#--------------------------------------------------------
#
#--------------------------------------------------------
__level2_isr:
                move.b  #0x04, (REG_IPEND_CLR).w
                rte                

#--------------------------------------------------------
#
#--------------------------------------------------------

#--------------------------------------------------------
# UART interrupt
#--------------------------------------------------------

                .extern uart_tx_ringbuf
                .extern uart_rx_ringbuf
                .extern uart_tx_size


__level3_isr:
                # Save registers
                movem.l d0-d7/a0-a6, -(sp)

                # Check for RXRDY interrupt pending
                btst    #B_UART_ISR_RXRDY, (REG_ISR).w
                beq.s   .check_tx_cause

                # RXRDY interrupt was pending
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
                bra.s   .exit

                #----------------------------------

                # Check for transmit interrupt
        .check_tx_cause:
                btst    #B_UART_ISR_TXEMT, (REG_ISR).w
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

#--------------------------------------------------------
# Interrupt handler for 1ms interval timer 
#--------------------------------------------------------

__level4_isr:
                st.b    (__interval_1ms_flag)
                movem.l d0-d7/a0-a6, -(sp)
                jsr     interval_1ms_handler
                movem.l (sp)+, d0-d7/a0-a6
                move.b  #0x10, (REG_IPEND_CLR).w
                rte                

                .extern __interval_1us_flag

#--------------------------------------------------------
# Interrupt handler for 1us interval timer 
#--------------------------------------------------------
__level5_isr:
                st.b    (__interval_1us_flag)
                movem.l d0-d7/a0-a6, -(sp)
                jsr     interval_1us_handler
                movem.l (sp)+, d0-d7/a0-a6
                move.b  #0x20, (REG_IPEND_CLR).w
                rte                


#--------------------------------------------------------
# Interrupt handler for 1ms system tick timer 
#--------------------------------------------------------
                .extern systick_handler
__level6_isr:
                st.b    (__systick_flag)
                movem.l d0-d7/a0-a6, -(sp)
                jsr     systick_handler
                movem.l (sp)+, d0-d7/a0-a6
                move.b  #0x40, (REG_IPEND_CLR).w
                rte

#--------------------------------------------------------
# Break button
#--------------------------------------------------------

__level7_isr:
                move.b  #0x80, (REG_IPEND_CLR).w
                st.b    (REG_DP3).w
                rte                

#--------------------------------------------------------
# Strings
#--------------------------------------------------------

                .align  16
msg_restart:
                .ascii  "Restarting BIOS ROM ...\n"
                dc.b    0
                .align  16
mainloopa:
                
                # Check if incoming data is ready
                jsr     uart_is_rxdata_ready
                bcc.s   .skip

                move.b  #'<', d0
                jsr     uart_putch                

                # Get data
                jsr     uart_getch
                jsr     uart_putch
# check
                move.b  #'>', d0
                jsr     uart_putch                
.skip:                

                # Check if there is USB data
                tst.b   (USB_STATUS).w
                bmi.s   .no_usb_data
                jsr     comms_check_dispatch
.no_usb_data:                


                move.b  (REG_STATUS).w, d0
                andi.b  #0x01, d0
                cmpi.w  #0x00, d0
                bne.s   .no_break

                jmp     reset
.no_break:
                jmp     mainloopa

#-----------------------------------------------------
# Strings
#-----------------------------------------------------

                .align  16
ansi_clrscr:
                .ascii  "\033[2J"
                .ascii  "\033[H"
                .align  16
msg_startup:
                .ascii  "*** SBC STARTUP ***\n\n"
                .ascii  "os>"
                dc.b    0
                .align  16

#-----------------------------------------------------
# Loopback test
#-----------------------------------------------------
uart_internal_loopback_test:
                
                move.b  #0x1A, (REG_CR).w
                move.b  #0x13, (REG_MR).w
                move.b  #0x87, (REG_MR).w
                move.b  #0x66, (REG_CSR).w
                move.b  #0x20, (REG_CR).w
                move.b  #0x30, (REG_CR).w
                move.b  #0x45, (REG_CR).w

                move.b  #0xAA, (REG_POST).w

                # send 256 chars

                moveq   #0, d0
next_char:
                # Send TX character
                move.b  d0, (REG_THR).w

                # Clear timeout
                moveq   #0, d6

                # Poll RXRDY status
poll_rxrdy:     move.b  (REG_SR).w, d7
                andi.b  #0x01, d7
                cmpi.b  #0x01, d7
                beq.s   poll_done

                # Bump wait count
                addq.l  #1, d6
                cmpi.l  #0x40000, d6
                bne.s   poll_rxrdy

                # Error: Timeout
                move.b  #0xBB, (REG_POST).w
                stop    #0x2700
poll_done:                
                # Get RX character and compare to TX character
                move.b  (REG_RHR).w, d1
                cmp.b   d1, d0
                bne.s   char_error

                # Test next character
                addq.b  #1, d0
                bne.s   next_char

                # Status: Test OK
                move.b  #0xC3, (REG_POST).w
                stop    #0x2700

char_error:                
                # Status: RX'd character mismatch
                move.b  #0x55, (REG_POST).w
                stop    #0x2700
                rts                

#-----------------------------------------------------
# Determine if data is ready to be read
# CF=1 : Data in RHR is ready to be read
# CF=0 : No data in RHR
#-----------------------------------------------------
uart_is_rxdata_ready:
                clc
                btst    #B_STATUS_RXRDY, (REG_SR).w
                beq.s   .done
                sec
.done:          rts

#-----------------------------------------------------
# Get character from SCC2691 UART
#-----------------------------------------------------
uart_getch:
                btst    #B_STATUS_RXRDY, (REG_SR).w
                beq.s   uart_getch
                move.b  (REG_RHR).w, d0
                rts

#-----------------------------------------------------
# Send character to SCC2691 UART
#-----------------------------------------------------
uart_putch:
                move.b  d0, (REG_THR).w
        .waitx: btst    #B_STATUS_TXRDY, (REG_SR).w
                beq.s   .waitx
                rts

#-----------------------------------------------------
# Put string
#-----------------------------------------------------
uart_puts:
                movem.l d0/a0, -(sp)
        .nexts:
                move.b  (a0)+, d0
                tst.b   d0
                beq.s   .dones
                jsr     uart_putch
                bra.s   .nexts
        .dones:
                movem.l (sp)+, d0/a0
                rts


#-----------------------------------------------------
# Program UART
#-----------------------------------------------------
uart_initialize:
                # Low commands
                move.b  #CR_DISABLE_TRANSMITTER, (REG_CR).w
                jsr     delay
                move.b  #CR_DISABLE_RECEIVER, (REG_CR).w
                jsr     delay

                # High commands
                move.b  #CR_RESET_MR, (REG_CR).w
                jsr     delay
                move.b  #CR_RESET_RECEIVER, (REG_CR).w
                jsr     delay
                move.b  #CR_RESET_TRANSMITTER, (REG_CR).w
                jsr     delay
                move.b  #CR_RESET_ERROR_STATUS, (REG_CR).w
                jsr     delay
                move.b  #CR_RESET_BREAK_CHANGE_INT, (REG_CR).w
                jsr     delay
                move.b  #CR_STOP_CT, (REG_CR).w
                jsr     delay
                move.b  #CR_RESET_MPI_CHANGE_INT, (REG_CR).w
                jsr     delay
                move.b  #CR_RESET_MR, (REG_CR).w
                jsr     delay

                # imr ; write only
                # isr ; read only
                # enable RxRDY/FFUL interrupt (bit 2)
                move.b  #0x04, (REG_IMR).w

# txemt
# interrupt when ready to send and buffer empty
# probably want to turn off transmitter when buffer is empty
# and turn it back on as soon as we put data into the buffer?



                # Set MR1 to 8-N-1
                move.b  #0b00010011, (REG_MR).w

                # Set MR2 to 1 stop bit
                move.b  #0b00000111, (REG_MR).w

                # Set first baud rate FF (Test 2)
                tst.b   (REG_BRG).w
                #tst.b   (REG_TEST).w (Test A)

                # Set baud rate to 1200 -> 115,200
                move.b  #0x66, (REG_CSR).w

                # Set BRG=0
                # Set PWRDN=off
                # Set TxC (1x) as MPO
                move.b  #0b00001011, (REG_ACR).w

                # Start TX and RX 
                move.b  #0x05, (REG_CR).w

                rts                

#-----------------------------------------------------
# Set POST digits
#-----------------------------------------------------
set_post:
                move.b  d0, (REG_POST).w
                rts

#-----------------------------------------------------
# Reset UART
#-----------------------------------------------------
uart_reset:
                move.b  #1, (REG_UART_RESET).w
                jsr     delay
                move.b  #0, (REG_UART_RESET).w
                jsr     delay

                rts

#-----------------------------------------------------
# Short delay
#-----------------------------------------------------
delay:
                move.l  d7, -(sp)
                moveq   #0x7F, d7
        wait_dly:
                nop                
                dbra    d7, wait_dly
                move.l  (sp)+, d7
                rts


#-------------------------------------------



                # Disable interrupts and set supervisor mode
                move.w  #0x2700, sr

                # Clear data registers
                moveq   #0, d0
                moveq   #0, d1
                moveq   #0, d2
                moveq   #0, d3
                moveq   #0, d4
                moveq   #0, d5
                moveq   #0, d6
                moveq   #0, d7

                # Clear address registers
                movea.l d0, a0
                movea.l d0, a1
                movea.l d0, a2
                movea.l d0, a3
                movea.l d0, a4
                movea.l d0, a5
                movea.l d0, a6

                # Set supervisor and user stack top
                lea     LOCAL_STACK_TOP, sp
                move    sp, usp

                
#---------------------------------------------------------------------
#---------------------------------------------------------------------
#---------------------------------------------------------------------




# read FF8041 reads status
# Fff normally
# fe when break pushed

# 9f plugged in
# ff not

# 1 rxf# = 1    not full
# 0 txe# = 0    is empty
# 0 pwe# = 0    is enum

# maybe try ramless echo?

# write abcd to ram.w
# read back 0xCD

# so does ram work or not?
# ram test fails


# txe=0         ok to send
#rxf=1          nothing to read

#txe=0                  ok to send
#rxf=0 after send       there's data to read

# BPL if N=0 (D7)
# BMI if N=1 (D7)
# BVS if V=1 (D6)
# BVC if V=0 (D6)

# read a23-a0
# read /as
# do mem download and look at addrs
#
#

                .macro  ugetb   dstreg
        1:      move.b  (USB_STATUS).w, d7
                andi.b  #0x80, d7
                cmpi.b  #0x80, d7
                beq.s   1b
                move.b  (USB_DATA).w, \dstreg
                .endm

                .macro usendb srcreg
        1:      move.b  (USB_STATUS).w, d7
                andi.b  #0x40, d7
                cmpi.b  #0x40, d7
                beq.s   1b
                move.b  \srcreg, (USB_DATA).w                
                .endm

                .macro ugetl    dstreg
                # get a31-a24
                ugetb   \dstreg
                lsl.l   #8, \dstreg
                # get a23-a16
                ugetb   \dstreg
                lsl.l   #8, \dstreg
                # get a15-a8
                ugetb   \dstreg
                lsl.l   #8, \dstreg
                # get a7-a0
                ugetb   \dstreg
                .endm

                .globl comms_check_dispatch
comms_check_dispatch:

                # Get command
                ugetb   d6

                cmpi.b  #0x00, d6
                beq.w   cmd_nop
                cmpi.b  #0x01, d6
                beq.w   cmd_reset
                cmpi.b  #0x02, d6
                beq.w   cmd_exit
                cmpi.b  #0x03, d6
                beq.w   cmd_exec
                cmpi.b  #0x04, d6
                beq.w   cmd_upload
                cmpi.b  #0x05, d6
                beq.w   cmd_download
                cmpi.b  #0x06, d6
                beq.w   cmd_peekb
                cmpi.b  #0x07, d6
                beq.w   cmd_peekw
                cmpi.b  #0x08, d6
                beq.w   cmd_peekl
                cmpi.b  #0x09, d6
                beq.w   cmd_pokeb
                cmpi.b  #0x0A, d6
                beq.w   cmd_pokew
                cmpi.b  #0x0B, d6
                beq.w   cmd_pokel
                cmpi.b  #0x0C, d6
                beq.w   cmd_download_nop
                cmpi.b  #0x0D, d6
                beq.w   cmd_echo
                cmpi.b  #0x0E, d6
                beq.w   cmd_id
                cmpi.b  #0x0F, d6
                beq.w   cmd_sync
                cmpi.b  #0x10, d6
                beq.w   cmd_upload_checksum
                cmpi.b  #0x11, d6
                beq.w   cmd_download_checksum
dispatch_next:                
                rts

#----------------------------------------------
cmd_nop:         
#----------------------------------------------
                nop
                bra.w   dispatch_next

#----------------------------------------------
cmd_reset:
#----------------------------------------------
                move.w  #0x2700, sr
                lea     LOCAL_STACK_TOP, sp
                lea     (0x0000).w, a0
                move.l  (a0)+, sp
                move    sp, usp
                move.l  (a0)+, a0
                jmp     (a0)

#----------------------------------------------
cmd_exit:        
#----------------------------------------------
                bra.w   dispatch_next

#----------------------------------------------
cmd_exec:                 
#----------------------------------------------
                # Get address      
                ugetl   d0
                move    d0, a0

                # Get mode
                ugetb   d0
                jmp     (a0)

#----------------------------------------------
cmd_upload:        
#----------------------------------------------
                # Get address
                ugetl   d4
                move.l  d4, a4
                # Get size
                ugetl   d5
                # Transfer loop
        cmd_upload_loop:
                ugetb   d0
                move.b  d0, (a4)+
                subq.l  #1, d5
                bne.s   cmd_upload_loop
                bra.w   dispatch_next

#----------------------------------------------
cmd_download:   
#----------------------------------------------
                # Get address
                ugetl   d4
                move.l  d4, a4
                # Get size
                ugetl   d5
                # Transfer loop
        cmd_download_loop:
                move.b  (a4)+, d0
                nop
                usendb  d0
                subq.l  #1, d5
                bne.s   cmd_download_loop
                bra.w   dispatch_next


                bra.w   dispatch_next

#----------------------------------------------
cmd_peekb:
#----------------------------------------------
                bra.w   dispatch_next
#----------------------------------------------
cmd_peekw:
#----------------------------------------------
                bra.w   dispatch_next
#----------------------------------------------
cmd_peekl:
#----------------------------------------------
                bra.w   dispatch_next
#----------------------------------------------
cmd_pokeb:
#----------------------------------------------
                bra.w   dispatch_next
#----------------------------------------------
cmd_pokew:
#----------------------------------------------
                bra.w   dispatch_next
#----------------------------------------------
cmd_pokel:
#----------------------------------------------
                bra.w   dispatch_next
#----------------------------------------------
cmd_download_nop:
#----------------------------------------------
                bra.w   dispatch_next
#----------------------------------------------
cmd_echo:
#----------------------------------------------
                ugetb   d6
                usendb  d6
                move.b  d6, d0
                jsr     uart_putch
                cmpi.b  #0x1B, d6
                bne.s   cmd_echo
                bra.w   dispatch_next
#----------------------------------------------
cmd_id:
#----------------------------------------------

                move.b  #'F', d0
                usendb  d0
                bra.w   dispatch_next
#----------------------------------------------
cmd_sync:
#----------------------------------------------
                bra.w   dispatch_next
#----------------------------------------------
cmd_upload_checksum:
#----------------------------------------------
                bra.w   dispatch_next
#----------------------------------------------
cmd_download_checksum:
#----------------------------------------------

                bra.w   dispatch_next

                .globl hard_fault
hard_fault:
                lea     msg_hard_fault, a0
                jsr     uart_puts

                # Force double fault
                lea     1, sp
                tst.l   1

                # Never reached
                stop    #0x2700

msg_hard_fault:
                .ascii  "*** HARD FAULT ***"
                dc.b    0                

                .align  16

                .globl soft_reset
soft_reset:
                # Disable interrupts
                move.w  #0x2700, sr

                # Turn off timers
                move.b  #0, (REG_TIMER_1US_ENA)
                move.b  #1, (REG_TIMER_1US_ENA)

                # Disable all interrupts
                move.b  #0x00, (REG_IENABLE).w

                # Clear all pending interrupt states
                move.b  #0xFF, (REG_IPEND_CLR).w

                # Restart program in FLASH
                reset
                jmp     0x180000

                .globl soft_reboot
soft_reboot:
                # Disable interrupts
                move.w  #0x2700, sr

                # Turn off timers
                move.b  #0, (REG_TIMER_1US_ENA)
                move.b  #1, (REG_TIMER_1US_ENA)

                # Disable all interrupts
                move.b  #0x00, (REG_IENABLE).w

                # Clear all pending interrupt states
                move.b  #0xFF, (REG_IPEND_CLR).w

                # Restart program in FLASH
                lea     0x0000.w, a0
                move.l  (a0)+, sp
                move.l  sp, usp
                move.l  (a0)+, a0  
                reset
                jmp     (a0)              

                .globl get_sp
get_sp:                
                move.l  sp, d0
                rts
                .globl get_bp
get_bp:                
                move.l  a6, d0
                rts


__default_isr:
                move.b  #0xC9, (0xFFFF8001).l
                stop    #0x2700
                rte
__default_lv7:
                st.b    (0x00100800).l
                rte


#------------------------------------------------------------------------------
#
#------------------------------------------------------------------------------

# d0 is return value
# d0,d1,a0,a1 scratch, don't have to preserve

                # void target_get_state(target_state *p)
                .globl  target_read_state

# test to do unpack
target_read_state:
                # Get pointer to structure in A1
                move.l  4(sp), a0       
                move.l  8(sp), a1       

                # Copy state into structure (adl,adm,adh,dbl)
                movep.l 0x01(a0), d0
                move.l  d0, (a1)+

                # Copy state into structure (dbh,ctil,ctih,misc)
                movep.l 0x09(a0), d0
                move.l  d0, (a1)+

                rts

#------------------------------------------------------------------------------
# End
#------------------------------------------------------------------------------
