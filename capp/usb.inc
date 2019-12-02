#-------------------------------------------------------------------------------
# USB library
#-------------------------------------------------------------------------------
# BMI is N=1
# BPL is N=0

		.macro	usb_cmd	cmd:req
	.poll_txe\@:
		tst.b	(STATUS_TXE).w
		bmi.b	.poll_txe\@
		move.b	#\cmd, (USB_DATA).w
		.endm

# Get byte
		.macro	getb	dreg
	.poll_rxf\@:
		tst.b	(STATUS_RXF).w
		bmi.s	.poll_rxf\@
		move.b	(USB_DATA).w, \dreg
		.endm		

# Send byte
		.macro	sendb	dreg
	.poll_txe\@:
		tst.b	(STATUS_TXE).w
		bmi.b	.poll_txe\@
		move.b	\dreg, (USB_DATA).w
		.endm		

# Wait for enumeration
usb_wait_enum:
.poll_enum:
		# Poll ENUM# until it goes low (enumerated)
		tst.b	(STATUS_ENUM).w
		bmi.s	.poll_enum
		rts

#-------------------------------------------------------------------------------
# Byte access
#-------------------------------------------------------------------------------

# Get byte from host
usb_getb:
		getb	d0
		rts

# Send byte to host
usb_sendb:
		sendb	d0
		rts

# Exchange byte with host
usb_exchange:
		sendb	d0
		getb	d0
		rts
		

#-------------------------------------------------------------------------------
# Word access
#-------------------------------------------------------------------------------

# Send word to host
usb_sendw:
		move.w	d0, -(a7)
		sendb	d0
		ror.w	#8, d0
		sendb	d0
		move.w	(a7)+, d0
		rts

# Get word from host
usb_getw:
		getb	d0
		rol.w	#8, d0
		getb	d0
		rts	
			
#-------------------------------------------------------------------------------
# Longword access
#-------------------------------------------------------------------------------
		
# Send longword to host
usb_sendl:	
		move.l	d0, -(a7)
		sendb	d0
		ror.l	#8, d0
		sendb	d0
		ror.l	#8, d0
		sendb	d0
		ror.l	#8, d0
		sendb	d0
		move.l	(a7)+, d0
		rts

# Get longword from host
usb_getl:
		getb	d0
		lsl.l	#8, d0
		getb	d0
		lsl.l	#8, d0
		getb	d0
		lsl.l	#8, d0
		getb	d0
		rts

#------------------------------------------------------------------------------
# Fast transfer routines
#------------------------------------------------------------------------------

# Send byte
		.macro	isend 	dreg, statreg, datareg
	.\@:	tst.b	(\statreg)
		bmi.s	.\@
		move.b	\dreg, (\datareg)
		.endm

# Send byte with checksum
		.macro	isendc 	dreg, statreg, datareg, sumreg
	.\@:	tst.b	(\statreg)
		bmi.s	.\@
		move.b	\dreg, (\datareg)
		add.b	\dreg, \sumreg
		.endm
# Get byte
		.macro	iget	dreg, statreg, datareg
	.\@:	tst.b	(\statreg)
		bmi.s	.\@
		move.b	(\datareg), \dreg
		.endm

# Get byte with checksum
		.macro	igetc	dreg, statreg, datareg, sumreg
	.\@:	tst.b	(\statreg)
		bmi.s	.\@
		move.b	(\datareg), \dreg
		add.b	\dreg, \sumreg
		.endm
# Send block
		.macro	isendblock dreg, statreg, datareg
		rol.l	#8, \dreg
		isend	\dreg, \statreg, \datareg
		rol.l	#8, \dreg
		isend	\dreg, \statreg, \datareg
		rol.l	#8, \dreg
		isend	\dreg, \statreg, \datareg
		rol.l	#8, \dreg
		isend	\dreg, \statreg, \datareg
		.endm

# Send block with checksum
		.macro	isendblockc dreg, statreg, datareg, sumreg
		rol.l	#8, \dreg
		isendc	\dreg, \statreg, \datareg, \sumreg
		rol.l	#8, \dreg
		isendc	\dreg, \statreg, \datareg, \sumreg
		rol.l	#8, \dreg
		isendc	\dreg, \statreg, \datareg, \sumreg
		rol.l	#8, \dreg
		isendc	\dreg, \statreg, \datareg, \sumreg
		.endm
# Get block
		.macro	igetblock dreg, statreg, datareg
		iget	\dreg, \statreg, \datareg
		rol.l	#8, \dreg
		iget	\dreg, \statreg, \datareg
		rol.l	#8, \dreg
		iget	\dreg, \statreg, \datareg
		rol.l	#8, \dreg
		iget	\dreg, \statreg, \datareg
		.endm

# Get block with checksum
		.macro	igetblockc dreg, statreg, datareg, sumreg
		igetc	\dreg, \statreg, \datareg, \sumreg
		rol.l	#8, \dreg
		igetc	\dreg, \statreg, \datareg, \sumreg
		rol.l	#8, \dreg
		igetc	\dreg, \statreg, \datareg, \sumreg
		rol.l	#8, \dreg
		igetc	\dreg, \statreg, \datareg, \sumreg
		.endm

#-------------------------------------------------------------------------------
# Get data from host
#-------------------------------------------------------------------------------
# d0 = Transfer size in bytes
# a0 = Buffer address
usb_get:
		push	d0-d7/a0-a6

		# Save size
		move.l	d0, d1

		# Point to USB registers
		lea	(STATUS_RXF).w, a4
		lea	(USB_DATA).w, a5

		# Transfer size is zero bytes, abort
		tst.l	d0
		beq.w	.ul_done

		# Determine transfer size in 16-byte blocks
		lsr.l	#4, d0
		tst.l	d0
		beq.w	.ul_check_remainder

		# Send blocks
.ul_send_blocks:
		igetblock	d4, a4, a5
		igetblock	d5, a4, a5
		igetblock	d6, a4, a5
		igetblock	d7, a4, a5
		movem.l	d4-d7, (a0)
		lea	0x10(a0), a0
		subq.l	#1, d0
		bne.w	.ul_send_blocks

		# Check for remainder
.ul_check_remainder:
		moveq	#0x0F, d0
		and.l	d1, d0		
		beq.s	.ul_done

		# Send remainder
.ul_send_remainder:		
		iget	d4, a4, a5
		move.b	d4, (a0)+
		subq.l	#1, d0
		bne.s	.ul_send_remainder		
.ul_done:
		pop	d0-d7/a0-a6
		rts

#-------------------------------------------------------------------------------
# Get data from host with checksum
#-------------------------------------------------------------------------------
# d0 = Transfer size in bytes
# a0 = Buffer address
usb_get_checksum:
		push	d0-d7/a0-a6

		# Save size
		move.l	d0, d1

		# Zero checksum
		moveq	#0, d2

		# Point to USB registers
		lea	(STATUS_RXF).w, a4
		lea	(USB_DATA).w, a5

		# Transfer size is zero bytes, abort
		tst.l	d0
		beq.w	.ulc_send_checksum	

		# Determine transfer size in 16-byte blocks
		lsr.l	#4, d0
		tst.l	d0
		beq.w	.ulc_check_remainder

		# Send blocks
.ulc_send_blocks:
		igetblockc	d4, a4, a5, d2
		igetblockc	d5, a4, a5, d2
		igetblockc	d6, a4, a5, d2
		igetblockc	d7, a4, a5, d2
		movem.l		d4-d7, (a0)
		lea		0x10(a0), a0
		subq.l		#1, d0
		bne.w		.ulc_send_blocks

		# Check for remainder
.ulc_check_remainder:
		moveq	#0x0F, d0
		and.l	d1, d0		
		beq.s	.ulc_send_checksum

		# Send remainder
.ulc_send_remainder:		
		igetc	d4, a4, a5, d2
		move.b	d4, (a0)+
		subq.l	#1, d0
		bne.s	.ulc_send_remainder		

		# Send checksum
.ulc_send_checksum:
		move.b	d2, d0
		jsr	usb_sendb

		pop	d0-d7/a0-a6
		rts

#-------------------------------------------------------------------------------
# Send data to host with checksum
#-------------------------------------------------------------------------------
# d0 = Transfer size in bytes
# a0 = Buffer address
usb_send:
		push	d0-d7/a0-a6

		# Save size
		move.l	d0, d1

		# Point to USB registers
		lea	(STATUS_TXE).w, a4
		lea	(USB_DATA).w, a5

		# Transfer size is zero bytes, abort
		tst.l	d0
		beq.w	.dl_done

		# Determine transfer size in 16-byte blocks
		lsr.l	#4, d0
		tst.l	d0
		beq.w	.dl_check_remainder

		# Test for last block
		subq.l	#1, d0
		tst.l	d0
		beq.w	.dl_transfer_last_block

		# Send blocks
.dl_send_blocks:
		movem.l		(a0)+, d4-d7
		isendblock	d4, a4, a5
		isendblock	d5, a4, a5
		isendblock	d6, a4, a5
		isendblock	d7, a4, a5
		subq.l		#1, d0
		bne.w		.dl_send_blocks

		# Send last block
.dl_transfer_last_block:
		movem.l		(a0)+, d4-d6
		move.l		(a0)+, d7	/* NOTE: MOVEM read quirk */
		isendblock	d4, a4, a5
		isendblock	d5, a4, a5
		isendblock	d6, a4, a5
		isendblock	d7, a4, a5

		# Check for remainder
.dl_check_remainder:
		moveq	#0x0F, d0
		and.l	d1, d0		
		beq.s	.dl_done

		# Send remainder
.dl_send_remainder:		
		move.b	(a0)+, d4
		isend	d4, a4, a5
		subq.l	#1, d0
		bne.s	.dl_send_remainder		
.dl_done:
		pop	d0-d7/a0-a6
		rts

#-------------------------------------------------------------------------------
# Send data to host with checksum
#-------------------------------------------------------------------------------
# d0 = Transfer size in bytes
# a0 = Buffer address
usb_send_checksum:
		push	d0-d7/a0-a6

		# Save size
		move.l	d0, d1

		# Zero checksum
		moveq	#0, d2

		# Point to USB registers
		lea	(STATUS_TXE).w, a4
		lea	(USB_DATA).w, a5

		# Transfer size is zero bytes, abort
		tst.l	d0
		beq.w	.dlc_send_checksum	

		# Determine transfer size in 16-byte blocks
		lsr.l	#4, d0
		tst.l	d0
		beq.w	.dlc_check_remainder

		# Test for last block
		subq.l	#1, d0
		tst.l	d0
		beq.w	.dlc_transfer_last_block

		# Send blocks
.dlc_send_blocks:
		movem.l		(a0)+, d4-d7
		isendblockc	d4, a4, a5, d2
		isendblockc	d5, a4, a5, d2
		isendblockc	d6, a4, a5, d2
		isendblockc	d7, a4, a5, d2
		subq.l		#1, d0
		bne.w		.dlc_send_blocks

		# Send last block
.dlc_transfer_last_block:
		movem.l		(a0)+, d4-d6
		move.l		(a0)+, d7	/* NOTE: MOVEM read quirk */
		isendblockc	d4, a4, a5, d2
		isendblockc	d5, a4, a5, d2
		isendblockc	d6, a4, a5, d2
		isendblockc	d7, a4, a5, d2

		# Check for remainder
.dlc_check_remainder:
		moveq	#0x0F, d0
		and.l	d1, d0		
		beq.s	.dlc_send_checksum

		# Send remainder
.dlc_send_remainder:		
		move.b	(a0)+, d4
		isendc	d4, a4, a5, d2
		subq.l	#1, d0
		bne.s	.dlc_send_remainder		

		# Send checksum
.dlc_send_checksum:
		move.b	d2, d0
		jsr	usb_sendb

		pop	d0-d7/a0-a6
		rts

#-------------------------------------------------------------------------------
# End
#-------------------------------------------------------------------------------

