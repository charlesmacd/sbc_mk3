#------------------------------------------------------------------------------
# PIC flash
#------------------------------------------------------------------------------

				.include "macros.inc"

#------------------------------------------------------------------------------
# Library entry point
#------------------------------------------------------------------------------

                .global pic_flash_library_base
pic_flash_library_base:
                bra.w   pic_flash_write
                bra.w   pic_flash_read
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error
                bra.w   pic_flash_error

#------------------------------------------------------------------------------
# Flash routines
#------------------------------------------------------------------------------

pic_flash_error:
                move.l  #0x4e564a47, 0x104000
                rts

# Input
# A0 = Offset in flash to write to
# Output
# A4 = (offset & ~0xFFFF) | 0x5555 << 1
# A5 = (offset & ~0xFFFF) | 0x2AAA << 1
pic_flash_pointer_setup:

                # Get flash address with A15-A0 reset
                move.l  a0, d0
                clr.w   d0                

                # Point A4 to flash command offset #1
                lea     (0x5555 << 1), a4
                lea     (a4, d0), a4

                # Point A5 to flash command offset #2
                lea     (0x2AAA << 1), a5
                lea     (a5, d0), a5
                rts

# Enter ID mode
pic_flash_cmd_id_sw_entry:
                move.w  #0xAAAA, (a4)
                move.w  #0x5555, (a5)
                move.w  #0x9090, (a4)
                rts                

# Exit ID mode
pic_flash_cmd_id_sw_exit:
                move.w  #0xAAAA, (a4)
                move.w  #0x5555, (a5)
                move.w  #0xF0F0, (a4)
                rts                

# Program flash page
pic_flash_cmd_program:
                move.w  #0xAAAA, (a4)
                move.w  #0x5555, (a5)
                move.w  #0xA0A0, (a4)
                rts 

# Delay for 1ms
pic_flash_delay:
#                move.l  d7, -(sp)
 #               move.w  #0x7FFF, d7
  #  fdly:       nop
   #             dbra    d7, fdly
    #            move.l  (sp)+, d7
                rts
                            

#-----------------------------------------------------------------
# Timeout is 2^31 cycles
#-----------------------------------------------------------------
# d0 is wait count
# d5 is 0x4040 mask
# d6 is temp storage
# d7 is temp storage
pic_flash_wait_toggle:
                movem.l d0/d5-d7, -(sp)
                move.w  #0x4040, d5
                move.l  #0x7FFFFFFF, d0
                move.w  (a0), d6
                and.w   d5, d6
    .wait_toggle:                
                move.w  (a0), d7
                and.w   d5, d7
                cmp.w   d6, d7
                beq.s   .wait_done
                move.w  d7, d6
                subq.l  #1, d0
                bne.s   .wait_toggle                            
    .wait_timeout:
                movem.l (sp)+, d0/d5-d7   
                sec
                rts
    .wait_done:           
                movem.l (sp)+, d0/d5-d7
                clc
                rts

# pic_flash_read(uint32_t address, uint32_t num_pages, uint8_t *data)
pic_flash_read:
                # Point A0 to parameters past return address
                lea     4(sp), a0

                # Save registers
                movem.l d0-d7/a0-a7, -(sp)

                # Get parameters
                move.l  8(a0), a3
                move.l  4(a0), d1
                move.l  0(a0), a0

                # Prepare to send flash commands
                bsr.w   pic_flash_pointer_setup

                # Send software ID entry command
                bsr.w   pic_flash_cmd_id_sw_entry
                bsr.w   pic_flash_delay

                # Copy data out of flash to buffer
    .copy_flash:                
                move.w  (a0)+, (a3)+
                subq.l  #1, d1
                bne.s   .copy_flash

                # Send software ID exit command
                bsr.w   pic_flash_cmd_id_sw_exit
                bsr.w   pic_flash_delay

                # Restore registers
                movem.l (sp)+, d0-d7/a0-a7
                rts

# pic_flash_write(uint32_t address, uint32_t num_pages, uint8_t *data)
pic_flash_write:
                # Point A0 to parameters past return address
                lea     4(sp), a0

                # Save registers
                movem.l d0-d7/a0-a7, -(sp)

                # Get parameters
                move.l  8(a0), a3
                move.l  4(a0), d1
                move.l  0(a0), a0

                # Prepare to send flash commands
                bsr.w   pic_flash_pointer_setup

                # Issue program command
                bsr.w   pic_flash_cmd_program

                # Write 128 bytes (64 words) to page buffer
                moveq   #(128/2)-1, d7
    .load_page: move.w  (a3)+, (a0)+
                dbra    d7, .load_page

                # Wait after page buffer has been loaded
                bsr.w   pic_flash_delay

                # Point back to last entry in page buffer
                lea     -2(a0), a0

                # Wait for toggle timer
                bsr.w   pic_flash_wait_toggle
                bcc     .page_written

                # Error, page not written
                nop
.page_written:                
                # Delay after page program dne
                bsr.w   pic_flash_delay

                # Restore registers
                movem.l (sp)+, d0-d7/a0-a7

                rts

#------------------------------------------------------------------------------
# Data
#------------------------------------------------------------------------------

format:
                .ascii  "VALUE[%08X,%08X,%08X]\n"
                dc.b    0,0

#------------------------------------------------------------------------------
# End
#------------------------------------------------------------------------------
