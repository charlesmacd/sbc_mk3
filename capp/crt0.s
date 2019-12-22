#------------------------------------------------------------------------------
# crt0.s
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
                .include "cpu_asm.inc"
                .include "printf_asm.inc"

#------------------------------------------------------------------------------
# Entry point
#------------------------------------------------------------------------------

#                printf "PC:%04X", d0, d1, d2, sr, 1234, print_formatted, 1,2 , 3, 4, 5, 6

reset:
                # Set up stack
                move.w  #0x2700, sr              
                lea     LOCAL_STACK_TOP, a7
                move.l  a7, usp                

                # Set up interrupt redirection table
                jsr     patch_interrupt_redirection_table

                # Run main function 
                jsr     main

                # Tell user main() has exited and we are restarting
                lea     msg_restart, a0
                jsr     uart_puts_polling

                # Re-run boot ROM
                jmp     reset_from_rom

#------------------------------------------------------------------------------
#
#------------------------------------------------------------------------------
terminal_clrscr:
                lea     ansi_clrscr, a0
                jsr     uart_puts_polling
                rts

#------------------------------------------------------------------------------
# Patch interrupt redirection table with a new vector
#------------------------------------------------------------------------------
# d0 = number
# a0 = address
register_isr:
                movem.l d0/a0-a1, -(sp)
                
                # Compute A1 = ISR_BASE + (number * 8) + 2
                lsl.l   #3, d0
                addq.l  #2, d0
                lea     ISR_BASE, a1
                
                # Patch table with new ISR address
                move.l  a0, (a1, d0.w)

                movem.l (sp)+, d0/a0-a1
                rts

#------------------------------------------------------------------------------
# Patch all autovectors
#------------------------------------------------------------------------------
patch_interrupt_redirection_table:

                # Patch interrupt redirection table
                move.w  #(256-2)-1, d1
                moveq   #2, d0
                lea     generic_exception_handler, a0
        .setup_isrs_default:   
                jsr     register_isr
                addq.b  #1, d0
                dbra    d1, .setup_isrs_default


                # Patch interrupt redirection table
                lea     .isr_table, a1
                moveq   #(7)-1, d1
        .setup_isrs:                
                move.l  (a1)+, d0
                move.l  (a1)+, a0
                jsr     register_isr
                dbra    d1, .setup_isrs

                rts

                .align  4
        .isr_table:

                # Unused
                dc.l    AUTOVECTOR_LEVEL1_NUMBER, __level1_isr
                # Unused
                dc.l    AUTOVECTOR_LEVEL2_NUMBER, __level2_isr
                # SCC2691 USART
                dc.l    AUTOVECTOR_LEVEL3_NUMBER, __level3_isr
                # 1ms timer
                dc.l    AUTOVECTOR_LEVEL4_NUMBER, __level4_isr                
                # 1us timer
                dc.l    AUTOVECTOR_LEVEL5_NUMBER, __level5_isr
                # Systick timer (1ms)
                dc.l    AUTOVECTOR_LEVEL6_NUMBER, __level6_isr
                # Break button
                dc.l    AUTOVECTOR_LEVEL7_NUMBER, __level7_isr

#------------------------------------------------------------------------------
# ???
#------------------------------------------------------------------------------
__level1_isr:
                move.b  #0x02, (REG_IPEND_CLR).w
                rte                
#------------------------------------------------------------------------------
# ???
#------------------------------------------------------------------------------
__level2_isr:
                move.b  #0x04, (REG_IPEND_CLR).w
                rte                
#------------------------------------------------------------------------------
# UART interrupt
#------------------------------------------------------------------------------
                .extern test_isr
                .extern handler_isr
__level3_isr:
                jmp     handler_isr

#--------------------------------------------------------
# Interrupt handler for 1ms interval timer 
#--------------------------------------------------------
__level4_isr:
                tas.b    (__interval_1ms_flag)
                movem.l d0-d7/a0-a6, -(sp)
                jsr     interval_1ms_handler
                movem.l (sp)+, d0-d7/a0-a6
                move.b  #0x10, (REG_IPEND_CLR).w
                rte                

#--------------------------------------------------------
# Interrupt handler for 1us interval timer 
#--------------------------------------------------------
__level5_isr:
                tas.b    (__interval_1us_flag)
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
                tas.b    (__systick_flag)
                addi.l  #1, (__systick_count)
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
# Set POST digits
#-----------------------------------------------------
set_post:
                move.b  d0, (REG_POST).w
                rts

#-------------------------------------------------------------------------------
#
#-------------------------------------------------------------------------------

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
                jmp     LOAD_ADDRESS


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
                jmp     reset_from_rom

#------------------------------------------------------------------------------
#
#------------------------------------------------------------------------------

                .section .KERNELVARS  

ex_pc:          ds.l    1
ex_sp:          ds.l    1
ex_sr:          ds.l    1
ex_ir:          ds.l    1
ex_access:      ds.l    1
ex_fc:          ds.l    1
ex_is_group0:   ds.l    1
ex_number       =       ex_pc           /* PC[31:24] */
                .section .text
                

generic_exception_handler:

                # Save stack pointer
                move.l sp, (ex_sp).l

                # Save stacked SR 
                move.l (sp)+, (ex_sr).l

                # Save stacked PC
                move.l (sp)+, (ex_pc).l

                # Check exception group type
                clr.b   (ex_is_group0).l
                cmpi.b  #0x02, (ex_number).l
                bgt.s   .is_group12
.is_group0:
                # Save instruction register
                move.w  (sp)+, (ex_ir).l

                # Save access address 
                move.w  (sp)+, (ex_access).l

                # Save function code
                move.w  (sp)+, (ex_fc).l

                # Flag as a group 0 exception
                st.b    (ex_is_group0).l
.is_group12:



                # Clear screen
###                jsr     terminal_clrscr

                # Print message
                lea     msg0, a0
                jsr     uart_puts_polling
                jsr     newline

                # Print PC:
                move.l  (ex_pc).l, d0
                jsr     printhexl
                jsr     newline

                # Print SR:
                move.l  (ex_sr).l, d0
                jsr     printhexw
                jsr     newline

                tst.b   (ex_is_group0).l 
                beq.s   .skip_print_group0

                # Print IR
                move.l  (ex_ir).l, d0
                jsr     printhexw
                jsr     newline

                # Print access address
                move.l  (ex_access).l, d0
                jsr     printhexl
                jsr     newline

                # Print function code
                move.l  (ex_fc).l, d0
                jsr     printhexw
                jsr     newline


.skip_print_group0:

                # Print message
                lea     msg1, a0
                jsr     uart_puts_polling
                jsr     newline

        .ex_loop:
                jsr     comms_check_dispatch
                bra.s   .ex_loop

                rte


msg0:           .asciz  "Generic exception occurred."
msg1:           .asciz  "Waiting for comms ..."

#------------------------------------------------------------------------------
# End
#------------------------------------------------------------------------------

