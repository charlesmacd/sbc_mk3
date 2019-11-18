#-------------------------------------------------------------------------------
# comms_asm.s --
#-------------------------------------------------------------------------------

                .include "macros.inc"
                .include "hw_defs.inc"

                .extern uart_putch

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
                jmp     reset_from_rom

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
                jsr     uart_putch_polling 
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

#-------------------------------------------------------------------------------
#
#-------------------------------------------------------------------------------
