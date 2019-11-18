#-------------------------------------------------------------------------------
# cpu_asm.s --
#-------------------------------------------------------------------------------

                .include "macros.inc"
                .include "hw_defs.inc"
                .include "cpu_asm.inc"

#-------------------------------------------------------------------------------
# Get stack pointer
#-------------------------------------------------------------------------------
                .globl get_sp
get_sp:                
                move.l  sp, d0
                rts

#-------------------------------------------------------------------------------
# Get base pointer
#-------------------------------------------------------------------------------
                .globl get_bp
get_bp:                
                move.l  a6, d0
                rts

#-------------------------------------------------------------------------------
# Get status register
#-------------------------------------------------------------------------------
                .globl get_sr
get_sr:                
                move.w  sr, d0
                rts

#-------------------------------------------------------------------------------
# Set status register
#-------------------------------------------------------------------------------
                .globl set_sr
set_sr:
                move.w  d0, sr
                rts

#------------------------------------------------------------------------------
# Reset from ROM
#------------------------------------------------------------------------------

                .global reset_from_rom
reset_from_rom:
                move.w  #0x2700, sr
                lea     LOCAL_STACK_TOP, sp
                lea     (VECTOR_TABLE_BASE).w, a0
                move.l  (a0)+, sp
                move    sp, usp
                move.l  (a0)+, a0
                jmp     (a0)
                
#-------------------------------------------------------------------------------
# Trigger a hard fault
#-------------------------------------------------------------------------------

                .globl trigger_hard_fault
trigger_hard_fault:

                # Print hard fault message to UART (no polling)
                lea     msg_hard_fault, a0
                jsr     uart_puts_polling

                # Force double fault in processor
                lea     1, sp
                tst.l   1

                # Never reached
spinloop:       stop    #0x2700
                bsr.s   spinloop

                .align  2
msg_hard_fault: .ascii  "*** HARD FAULT ***"
                dc.b    0                
                .align  2

#-------------------------------------------------------------------------------
# End
#-------------------------------------------------------------------------------
