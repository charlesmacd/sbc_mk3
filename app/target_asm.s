#-------------------------------------------------------------------------------
# target_asm.s --
#-------------------------------------------------------------------------------

                .include "macros.inc"
                .include "hw_defs.inc"


# d0 is return value
# d0,d1,a0,a1 scratch, don't have to preserve

                # void target_get_state(target_state *p)
                .globl  target_read_state

# test to do unpack
target_read_state2:
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


                # void target_pulse_clock(uint16_t count)
                .global target_pulse_ffck
target_pulse_ffck:
                lea     0x908000, a0
                move.l  4(sp), d0
                subq.w  #1, d0
repeat:         movep.w d0, 0(a0)
                dbra    d0, repeat
                rts                

                .globl test_data
                .align  16
test_data:
                .incbin "kick.bin"
                .align  16

#-------------------------------------------------------------------------------
#
#-------------------------------------------------------------------------------
