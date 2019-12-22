#-------------------------------------------------------------------------------
# cpu_asm.s --
#-------------------------------------------------------------------------------

                .include "macros.inc"
                .include "hw_defs.inc"
                .include "L0_Platform/cpu_asm.inc"
                
#-------------------------------------------------------------------------------
# Returns bit position of highest set bit in the value, or -1 if no bits set.
# uint32_t __ffs32(uint32_t value);
#-------------------------------------------------------------------------------
                .global     __ffs32
__ffs32:
                move.l  4(sp), d1
                moveq   #31, d0
    .scan:      btst    d0, d1
                bne.s   .found
                subq.l  #1, d0
                bpl.s   .scan
    .found:
                rts

#-------------------------------------------------------------------------------
# Returns number of set bits in the value.
# uint32_t __popcount32(uint32_t value);
#-------------------------------------------------------------------------------
                .global __popcount32
__popcount32:
                move.l  d2, -(sp)
                lea     popcounts(pc), a0
                move.l  8(sp), d1
                moveq   #0, d0
                .rept   8
                moveq   #0x0F, d2
                and.l   d1, d2
                add.b   (a0, d2.w), d0
                lsr.l   #4, d1
                .endr
                move.l  (sp)+, d2
                rts

popcounts:      dc.b    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4

#-------------------------------------------------------------------------------
# uint32_t __atomic_tas(uint8_t *ptr);
#-------------------------------------------------------------------------------
# Test and set byte location in memory (atomic operation)
# Returns original value of location bit 7 in D0:
# Sets D0 to
# 00000000 : False (bit 7 was reset prior to TAS)
# 00000001 : True  (bit 7 set set prior to TAS)
#-------------------------------------------------------------------------------
                .global __atomic_tas
__atomic_tas:
                move.l  4(sp), a0
                moveq   #0, d0
                tas     (a0)
                smi     d0
                neg.b   d0
                rts                

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
                move.w  4(sp), sr
                rts


#------------------------------------------------------------------------------
# uint16_t get_ipl(void);
#------------------------------------------------------------------------------
                .global get_ipl
get_ipl:
                move.w  sr, d0
                lsr.w   #8, d0
                andi.w  #0x0007, d0
                rts

#------------------------------------------------------------------------------
# void set_ipl(uint16_t level);
#------------------------------------------------------------------------------
                .global set_ipl
set_ipl:
                move.w  4(sp), d0
                moveq   #7, d1
                and.w   d0, d1
                lsl.w   #8, d1
                move.w  sr, d0
                andi.w  #0xF8FF, d0
                or.w    d1, d0
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



                .globl huread_page_auto
huread_page_auto:

                movem.l a0-a1, -(sp)                

                # Reset counter
                move.b  #0xFF, 0x904001

                # Read one page
                lea     0x90400F, a0
                lea     __kernel_page_buffer, a1

                .rept   256
                move.b  (a0), (a1)+
                .endr

                movem.l (sp)+, a0-a1
                rts








