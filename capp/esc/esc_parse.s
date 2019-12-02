# This is an generated file. Do not edit.
	.abort:
		rts
	escape_parser:
		moveq	#-1, d1
MapMatch0000:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$1B, d0
		beq.w	MapMatch0001
		bra.w	.abort
MapMatch0001:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$5B, d0
		beq.w	MapMatch0002
		bra.w	.abort
MapMatch0002:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$31, d0
		beq.w	MapMatch0007
		cmpi.b	#$32, d0
		beq.w	MapMatch0009
		cmpi.b	#$33, d0
		beq.w	MapMatch000B
		cmpi.b	#$34, d0
		beq.w	MapMatch000D
		cmpi.b	#$35, d0
		beq.w	MapMatch000F
		cmpi.b	#$36, d0
		beq.w	MapMatch0011
		cmpi.b	#$41, d0
		beq.w	MapMatch0003
		cmpi.b	#$42, d0
		beq.w	MapMatch0004
		cmpi.b	#$43, d0
		beq.w	MapMatch0006
		cmpi.b	#$44, d0
		beq.w	MapMatch0005
		bra.w	.abort
MapMatch0007:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$31, d0
		beq.w	MapMatch0013
		cmpi.b	#$32, d0
		beq.w	MapMatch0015
		cmpi.b	#$33, d0
		beq.w	MapMatch0017
		cmpi.b	#$34, d0
		beq.w	MapMatch0019
		cmpi.b	#$35, d0
		beq.w	MapMatch001B
		cmpi.b	#$37, d0
		beq.w	MapMatch001D
		cmpi.b	#$38, d0
		beq.w	MapMatch001F
		cmpi.b	#$39, d0
		beq.w	MapMatch0021
		cmpi.b	#$7E, d0
		beq.w	MapMatch0008
		bra.w	.abort
MapMatch0013:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch0014
		bra.w	.abort
MapMatch0014:
		moveq	#$0A, d0
		rts
MapMatch0015:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch0016
		bra.w	.abort
MapMatch0016:
		moveq	#$0B, d0
		rts
MapMatch0017:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch0018
		bra.w	.abort
MapMatch0018:
		moveq	#$0C, d0
		rts
MapMatch0019:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch001A
		bra.w	.abort
MapMatch001A:
		moveq	#$0D, d0
		rts
MapMatch001B:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch001C
		bra.w	.abort
MapMatch001C:
		moveq	#$0E, d0
		rts
MapMatch001D:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch001E
		bra.w	.abort
MapMatch001E:
		moveq	#$0F, d0
		rts
MapMatch001F:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch0020
		bra.w	.abort
MapMatch0020:
		moveq	#$10, d0
		rts
MapMatch0021:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch0022
		bra.w	.abort
MapMatch0022:
		moveq	#$11, d0
		rts
MapMatch0008:
		moveq	#$04, d0
		rts
MapMatch0009:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$30, d0
		beq.w	MapMatch0023
		cmpi.b	#$31, d0
		beq.w	MapMatch0025
		cmpi.b	#$33, d0
		beq.w	MapMatch0027
		cmpi.b	#$34, d0
		beq.w	MapMatch0029
		cmpi.b	#$7E, d0
		beq.w	MapMatch000A
		bra.w	.abort
MapMatch0023:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch0024
		bra.w	.abort
MapMatch0024:
		moveq	#$12, d0
		rts
MapMatch0025:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch0026
		bra.w	.abort
MapMatch0026:
		moveq	#$13, d0
		rts
MapMatch0027:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch0028
		bra.w	.abort
MapMatch0028:
		moveq	#$14, d0
		rts
MapMatch0029:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch002A
		bra.w	.abort
MapMatch002A:
		moveq	#$15, d0
		rts
MapMatch000A:
		moveq	#$05, d0
		rts
MapMatch000B:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch000C
		bra.w	.abort
MapMatch000C:
		moveq	#$06, d0
		rts
MapMatch000D:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch000E
		bra.w	.abort
MapMatch000E:
		moveq	#$07, d0
		rts
MapMatch000F:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch0010
		bra.w	.abort
MapMatch0010:
		moveq	#$08, d0
		rts
MapMatch0011:
		cmpa	a0, a1
		beq.w	.abort
		move.b	-(a0), d0
		cmpi.b	#$7E, d0
		beq.w	MapMatch0012
		bra.w	.abort
MapMatch0012:
		moveq	#$09, d0
		rts
MapMatch0003:
		moveq	#$00, d0
		rts
MapMatch0004:
		moveq	#$01, d0
		rts
MapMatch0006:
		moveq	#$03, d0
		rts
MapMatch0005:
		moveq	#$02, d0
		rts
/* End */