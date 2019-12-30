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
                .include "L0_Platform/cpu_asm.inc"
                .include "printf_asm.inc"

#------------------------------------------------------------------------------
# Entry point
#------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# printf_asm.inc
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# End
#-------------------------------------------------------------------------------

                .extern _text_start
                .extern _etext
                .extern _stext
reset:
                # Set up stack
                move.w  #0x2700, sr              
                lea     LOCAL_STACK_TOP, a7
                move.l  a7, usp                

                # Set up interrupt redirection table
                jsr     patch_interrupt_redirection_table

                # Do C runtime
                jsr     startup

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
                movem.l d0-d1/a0-a1, -(sp)

                move.l  d0, d1
                andi.l  #0x000000FF, d1
                lsl.l   #8, d1
                lsl.l   #8, d1
                lsl.l   #8, d1

                andi.w  #0x000000FF, d0
                lsl.w   #3, d0
                addq.w  #2, d0
                lea     ISR_BASE, a1
                lea     (a1, d0.w), a1

                move.l  a0, d0
                andi.l  #0x00FFFFFF, d0
                or.l    d1, d0
                move.l  d0, (a1)


                movem.l (sp)+, d0-d1/a0-a1
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
                .globl __level1_isr
__level1_isr:
                move.b  #0x02, (REG_IPEND_CLR).w
                rte                
#------------------------------------------------------------------------------
# ???
#------------------------------------------------------------------------------
                .globl __level2_isr
__level2_isr:
                move.b  #0x04, (REG_IPEND_CLR).w
                rte                
#------------------------------------------------------------------------------
# UART interrupt
#------------------------------------------------------------------------------
                .extern UartSCC2681_ISR
                .extern uart
                .globl __level3_isr
__level3_isr:
                movem.l d0-d7/a0-a7, -(sp)
                pea     uart
                jsr     UartSCC2681_ISR
                addq.l  #4, sp
                movem.l (sp)+, d0-d7/a0-a7
                rte

#--------------------------------------------------------
# Interrupt handler for 1ms interval timer 
#--------------------------------------------------------
                .globl __level4_isr
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
                .globl __level5_isr
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
                .globl __level6_isr
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

                .globl __level7_isr
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

ex_regsave:     ds.l    64
ex_pc:          ds.l    1
ex_sp:          ds.l    1
ex_sr:          ds.l    1
ex_ir:          ds.l    1
ex_access:      ds.l    1
ex_fc:          ds.l    1
ex_is_group0:   ds.l    1
ex_regs:        ds.l    64
ex_number:      ds.l    1
                .section .text
                

generic_exception_handler:

                move.l  #0xaaaaaaaa, (ex_pc)
                move.l  #0xaaaaaaaa, (ex_sp)
                move.l  #0xaaaaaaaa, (ex_access)
                move.w  #0x5555, (ex_sr)
                move.w  #0x5555, (ex_ir)
                move.w  #0x5555, (ex_fc)
                

                # Save stack pointer
                move.l  sp, (ex_sp).l

                move.l  a7, (ex_sp)

                # Save registers
                movem.l d0-d7/a0-a7, (ex_regs).l

                # Get MSB of PC into D0                                
                pea     0(pc)
                move.b  0(sp), (ex_number)
                addq.l  #4, sp

                cmpi.b  #0x03, (ex_number)
                bgt.s   .is_group12
.is_group0:
                # Adjust saved SP for the size of the exception stack frame
                addi.l  #8, (ex_sp)

                # Save function code
                move.w  (sp)+, (ex_fc).l

                # Save access address 
                move.l  (sp)+, (ex_access).l

                # Save instruction register
                move.w  (sp)+, (ex_ir).l
.is_group12:
                # Adjust saved SP for the size of the exception stack frame
                addi.l  #6, (ex_sp)

                # Save stacked SR 
                move.w (sp)+, (ex_sr).l

                # Save stacked PC
                move.l (sp)+, (ex_pc).l

                # Start new line for exception output                
                jsr     newline

                # Print common banner
                printf  "================================================================\n"
                printf  " EXCEPTION\n"
                printf  "================================================================\n"

                # Print group type
                cmpi.b  #0x03, (ex_number)
                bgt.s   .print_group0_banner
                printf  "Grp: $01 or $02\n"
                bra.s   .print_banner_done
.print_group0_banner:
                printf  "Grp: $00\n"
.print_banner_done:

                # Print exception number and description
                move.b  (ex_number), d0
                printf  "Num: $%02X [", d0
                jsr     lookup_exception_msg
                jsr     uart_puts_polling
                printf  "]\n"

                # Print PC and SR
                move.l  (ex_pc), d0
                printf  " PC: $%08X\n", d0
                move.w  (ex_sr), d0
                printf  " SR: $%04X\n", d0

                # Print extended stacked information
                move.b  (ex_number).l, d0
                cmpi.b  #0x03, d0
                bgt.w   .skip_extended

                move.w  (ex_ir).l, d0
                printf  " IR: $%04X\n", d0
                move.l  (ex_access).l, d0
                printf  " AC: $%08X\n", d0
                move.w  (ex_fc).l, d0
                printf  " FC: $%04X\n", d0
.skip_extended:

                # Print registers
                printf  "Register data:\n", d0
                movem.l (ex_regs), d0-d7/a0-a6
                printf  " D0: $%08X  A0: $%08X\n", d0, a0
                printf  " D1: $%08X  A1: $%08X\n", d1, a1
                printf  " D2: $%08X  A2: $%08X\n", d2, a2
                printf  " D3: $%08X  A3: $%08X\n", d3, a3
                printf  " D4: $%08X  A4: $%08X\n", d4, a4
                printf  " D5: $%08X  A5: $%08X\n", d5, a5
                printf  " D6: $%08X  A6: $%08X\n", d6, a6

                # Print stack pointers
                move.l  (ex_sp), d0
                printf  " D7: $%08X SSP: $%08X\n", d7, d0
                move.l  usp, a0
                printf  "USP: $%08X\n", a0

                printf  "Stack dump:\n"
                moveq   #0, d1
                move.l  (ex_sp), d0
                btst    #0, d0
                beq.s   .valid_stack

                printf  "Invalid stack offset (%08X)\n", d0
                bra.s   .stack_dump_done
    .valid_stack:                
                move.l  d0, a0
                moveq   #(8)-1, d7
    .print_stack:                
                move.l  (a0), d0
                printf  " $%08X (SP-%02X) : $%08X\n", a0, d1, d0
                addq.l  #4, a0
                addq.l  #4, d1
                dbra    d7, .print_stack
    .stack_dump_done:            
                # Print message
                printf  "================================================================\n"
                printf  " Status: Waiting for comms ...\n"
                printf  "================================================================\n"

                # Wait for comms commands
        .ex_loop:
                jsr     comms_check_dispatch
                bra.s   .ex_loop

                rte

#-------------------------------------------------------------------------------
# Data
#-------------------------------------------------------------------------------

# Exception descriptions
em00:           .asciz  "Invalid (ISP)"
em01:           .asciz  "Invalid (IPC)"
em02:           .asciz  "Bus error (Cycle terminated by BERR#)"
em03:           .asciz  "Address error (Unaligned access)"
em04:           .asciz  "Illegal instructions"
em05:           .asciz  "Division by zero"
em06:           .asciz  "CHK exception"
em07:           .asciz  "TRAPV exception"
em08:           .asciz  "Privilege violation"
em09:           .asciz  "Trace exception"
em0A:           .asciz  "Line A emulator"
em0B:           .asciz  "Line F emulator"
em0C:           .asciz  "Reserved C"
em0D:           .asciz  "Reserved D"
em0E:           .asciz  "Reserved E"
em0F:           .asciz  "Uninitialized interrupt"
em10_17:        .asciz  "Reserved 10-17"
em18:           .asciz  "Spurious interrupt"
em19:           .asciz  "Level 1 interrupt autovector"
em1A:           .asciz  "Level 2 interrupt autovector"
em1B:           .asciz  "Level 3 interrupt autovector"
em1C:           .asciz  "Level 4 interrupt autovector"
em1D:           .asciz  "Level 5 interrupt autovector"
em1E:           .asciz  "Level 6 interrupt autovector"
em1F:           .asciz  "Level 7 interrupt autovector"
em20_2F:        .asciz  "Trap 0 to F"
em30_3F:        .asciz  "Reserved 30-3F"
em40_FF:        .asciz  "User interrupt vectors 40-FF"            
                .align  2

# Exception message table                
emtbl00_1F:
                dc.l    em00, em01, em02, em03
                dc.l    em04, em05, em06, em07
                dc.l    em08, em09, em0A, em0B
                dc.l    em0C, em0D, em0E, em0F
                .rept   8
                dc.l    em10_17
                .endr
                dc.l    em18, em19, em1A, em1B
                dc.l    em1C, em1D, em1E, em1F

# Look up exception type based on number
# d0 = number (00-FF)
lookup_exception_msg:
                move.w  d0, -(sp)
                andi.l  #0xFF, d0
                lea     em40_FF, a0
                btst    #7, d0
                bne.s   .exit               /* D0 was 80-FF */          
                btst    #6, d0
                bne.s   .exit               /* D0 was 40-7F */
                btst    #5, d0
                beq.s   .process_lower      /* D0 was 00-1F
                lea     em30_3F, a0
                btst    #4, d0
                bne.s   .exit               /* D0 was 30-3F */
                lea     em20_2F, a0
                bra.s   .exit               /* D0 was 20-2F */
        .process_lower:
                andi.b  #0x1F, d0
                lsl.w   #2, d0
                lea     emtbl00_1F, a0
                move.l  (a0, d0.w), a0
        .exit:                
                move.w  (sp)+, d0
                rts

#------------------------------------------------------------------------------
# End
#------------------------------------------------------------------------------

                        