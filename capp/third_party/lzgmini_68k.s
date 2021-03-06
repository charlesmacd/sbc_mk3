# -*- mode: asm# tab-width: 8# indent-tabs-mode: t# -*-

#-------------------------------------------------------------------------------
# This file is part of liblzg.
#
# Copyright (c) 2010 Marcus Geelnard
#
# This software is provided 'as-is',without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications,and to alter it and redistribute it
# freely,subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented# you must not
#    claim that you wrote the original software. If you use this software
#    in a product,an acknowledgment in the product documentation would
#    be appreciated but is not required.
#
# 2. Altered source versions must be plainly marked as such,and must not
#    be misrepresented as being the original software.
#
# 3. This notice may not be removed or altered from any source
#    distribution.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Description
# -----------
# This is an assembly language implemention of the LZG decoder for the Motorola
# 680x0 line of processors. It is written in Motorola syntax, and compiles with
# vasm (http://sun.hasenbraten.de/vasm/) for instance, and should be easy to
# modify for any other 68k assembler.
#
# The resulting code is about 0.5 KB, and requires no extra memory (except for
# 68 bytes of stack space). The main decompression loop easily fits into the
# 256-byte sized cache of a 68020 processor.
#
# Performance-wise, the routine decompresses about 100-200 KB/s on a stock
# Amiga 500 (7 MHz 68000, slow RAM).
#-------------------------------------------------------------------------------


#-- PRIVATE --------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Constants
#-------------------------------------------------------------------------------
 
LZG_HEADER_SIZE		=	16
LZG_METHOD_COPY		=	0
LZG_METHOD_LZG1		=	1


#-------------------------------------------------------------------------------
# LUT for decoding the copy length parameter (-1)
#-------------------------------------------------------------------------------

_LZG_LENGTH_DECODE_LUT:
	dc.b	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
	dc.b	17,18,19,20,21,22,23,24,25,26,27,28,34,47,71,127
	.align	4

#-------------------------------------------------------------------------------
# _LZG_GetUINT32 - alignment independent reader for 32-bit integers
# a0 = in
# d0 = offset
# d1 = result
#-------------------------------------------------------------------------------

_LZG_GetUINT32:
	move.b	(a0,d0.w),d1
	asl.w	#8,d1
	move.b	1(a0,d0.w),d1
	swap	d1
	move.b	2(a0,d0.w),d1
	asl.w	#8,d1
	move.b	3(a0,d0.w),d1
	rts


#-- PUBLIC ---------------------------------------------------------------------

#-------------------------------------------------------------------------------
# LZG_Decode - Decode a compressed memory block
# a0 = in
# d0 = insize
# a1 = out
# d1 = outsize
# d2 = result (number of decompressed bytes, or zero upon failure)
#-------------------------------------------------------------------------------



#	.global LZG_DecodedSize
#LZG_DecodedSize:
#	movem.l	d1-d7/a0-a7, -(a7)
#	move.l	0x40(a7), a0 ; in 1
#	move.l	0x3C(a7), d0 ; insize
#	bsr.s	_LZG_DecodedSize
#	move.l	d1, d0
#	movem.l	(a7)+, d1-d7/a0-a7	
#	rts	


#__reg("d2") unsigned int LZG_Decode(
#    __reg("a0") const unsigned char *in,
#    __reg("d0") unsigned int insize,
#    __reg("a1") unsigned char *out,
#    __reg("d1") unsigned int outsize
#);


	.global	LZG_Decode

LZG_Decode:
	movem.l	d1-d7/a0-a7, -(a7)

	# 1a0000
	move.l	0x4C(a7), d1

	# 28000
	move.l	0x48(a7), a1

	# c85d
	move.l	0x44(a7), d0 

	# 83184
	move.l	0x40(a7), a0

	bsr.s	_LZG_Decode	

	

	move.l	d2, d0

	movem.l	(a7)+, d1-d7/a0-a7		
	rts


_LZG_Decode:

# Stack frame
encodedSize	=	0
decodedSize	=	4
_FRAME_SIZE	=	8

	movem.l	d0/d1/d3/d4/d5/d6/d7/a0/a1/a2/a3/a4/a5/a6,-(sp)
	lea.l	-_FRAME_SIZE(sp),sp

	# Remember caller arguments
	move.l	d0,d4
	move.l	d1,d5

	# Check magic ID
	cmp.l	#LZG_HEADER_SIZE,d0
	bcs	.fail
	cmp.b	#'L',(a0)
	bne	.fail
	cmp.b	#'Z',1(a0)
	bne	.fail
	cmp.b	#'G',2(a0)
	bne	.fail


	# Get header data
	moveq	#3,d0
	bsr.s	_LZG_GetUINT32
	move.l	d1,decodedSize(sp)
	moveq	#7,d0
	bsr.s	_LZG_GetUINT32
	move.l	d1,encodedSize(sp)
	moveq	#11,d0
	bsr.s	_LZG_GetUINT32
	move.l	d1,d6


	# Check sizes
	move.l	decodedSize(sp),d7
	cmp.l	d5,d7
	bhi	.fail


	# this is C8B0
	move.l	encodedSize(sp),d7
	add.l	#LZG_HEADER_SIZE,d7


	cmp.l	d4,d7
	bhi	.fail


	# Initialize the byte streams
	lea.l	(a0,d4.l),a2
	move.l	a1,a3
	add.l	decodedSize(sp),a3
	lea.l	LZG_HEADER_SIZE(a0),a0


	move.l	a1,a6

	# Nothing to do...?
	cmp.l	a0,a2
	beq.s	.done

	# Calculate and check checksum
	move.l	a0,a4
	move.l	encodedSize(sp),d0
	subq.l	#1,d0
	moveq	#1,d2
	moveq	#0,d1
	moveq	#0,d3
.csloop:
	move.b	(a4)+,d3
	add.w	d3,d2
	add.w	d2,d1
	dbf	d0,.csloop
	swap	d1
	move.w	d2,d1
	cmp.l	d6,d1
	bne.s	.fail

	# Check which method to use
	move.b	-1(a0),d7
	beq	.plaincopy
	cmp.b	#LZG_METHOD_LZG1,d7
	bne.s	.fail

	# Get the marker symbols
	lea.l	3(a0),a4
	cmp.l	a2,a4
	bcc.s	.fail
	move.b	(a0)+,d1
	move.b	(a0)+,d2
	move.b	(a0)+,d3
	move.b	(a0)+,d4

	lea.l	_LZG_LENGTH_DECODE_LUT(pc),a5

	# Main decompression loop
	move.l	#2056,d0
	cmp.l	a2,a0
.mainloop:
	bcc.s	.fail
	move.b	(a0)+,d7

	cmp.b	d1,d7
	beq.s	.marker1
	cmp.b	d2,d7
	beq.s	.marker2
	cmp.b	d3,d7
	beq.s	.marker3
	cmp.b	d4,d7
	beq.s	.marker4

.literal:
	cmp.l	a3,a1
	bcc.s	.fail
	move.b	d7,(a1)+
	cmp.l	a2,a0
	bcs.s	.mainloop

	# We're done
.done:
	cmp.l	a3,a1
	bne.s	.fail
	move.l	decodedSize(sp),d2
.exit:	
	lea.l	_FRAME_SIZE(sp),sp
	movem.l	(sp)+,d0/d1/d3/d4/d5/d6/d7/a0/a1/a2/a3/a4/a5/a6
	rts

	# This is where we end up if something went wrong...
.fail:
	moveq	#0,d2
	bra.s	.exit

	# marker4 - "Near copy (incl. RLE)"
.marker4:
	cmp.l	a2,a0
	bcc.s	.fail
	moveq	#0,d5
	move.b	(a0)+,d5
	beq.s	.literal
	move.l	d5,d6
	and.b	#0x1f,d6
	move.b	(a5,d6.w),d6
	lsr.b	#5,d5
	addq.w	#1,d5
	bra.s	.copy

	# marker3 - "Short copy"
.marker3:
	cmp.l	a2,a0
	bcc.s	.fail
	moveq	#0,d5
	move.b	(a0)+,d5
	beq.s	.literal
	move.l	d5,d6
	lsr.b	#6,d6
	addq.w	#2,d6
	and.b	#0x3f,d5
	addq.w	#8,d5
	bra.s	.copy

	# marker2 - "Medium copy"
.marker2:
	cmp.l	a2,a0
	bcc.s	.fail
	moveq	#0,d5
	move.b	(a0)+,d5
	beq.s	.literal
	cmp.l	a2,a0
	bcc.s	.fail
	move.l	d5,d6
	and.b	#0x1f,d6
	move.b	(a5,d6.w),d6
	lsl.w	#3,d5
	move.b	(a0)+,d5
	addq.w	#8,d5
	bra.s	.copy

	# marker1 - "Distant copy"
.marker1:
	cmp.l	a2,a0
	bcc.s	.fail
	moveq	#0,d5
	move.b	(a0)+,d5
	beq.s	.literal
	lea.l	1(a0),a4
	cmp.l	a2,a4
	bcc.s	.fail
	move.l	d5,d6
	and.b	#0x1f,d6
	move.b	(a5,d6.w),d6
	lsr.w	#5,d5
	swap	d5
	move.b	(a0)+,d5
	lsl.w	#8,d5
	move.b	(a0)+,d5
	add.l	d0,d5

	# Copy corresponding data from history window
	# d5 = offset
	# d6 = length-1
.copy:
	lea.l	(a1,d6.l),a4
	cmp.l	a3,a4
	bcc	.fail

	move.l	a1,a4
	sub.l	d5,a4

	cmp.l	a6,a4
	bcs	.fail

.loop1:	move.b	(a4)+,(a1)+
	dbf	d6,.loop1

	cmp.l	a2,a0
	bcs	.mainloop
	bra	.done

	# For non-compressible data, we copy the data as it is (1:1)
.plaincopy:
	move.l	encodedSize(sp),d6
	cmp.l	decodedSize(sp),d6
	bne	.fail
	subq.l	#1,d6
.loop2:	move.b	(a0)+,(a1)+
	dbf	d6,.loop2
	bra	.done



#-------------------------------------------------------------------------------
# LZG_DecodedSize - Determine the uncompressed size for compressed memory block
# a0 = in
# d0 = insize
# d1 = result (number of decompressed bytes, or zero upon failure)
#-------------------------------------------------------------------------------

	.global LZG_DecodedSize
LZG_DecodedSize:
	movem.l	d1-d7/a0-a7, -(a7)
	move.l	0x40(a7), a0
	move.l	0x3C(a7), d0
	bsr.s	_LZG_DecodedSize
	move.l	d1, d0
	movem.l	(a7)+, d1-d7/a0-a7	
	rts	
	
# Wrapper
_LZG_DecodedSize:
	move.l	d0,-(sp)

	# Check magic ID
	cmp.l	#7,d0
	bcs.s	.fail2
	cmp.b	#'L',(a0)
	bne.s	.fail2
	cmp.b	#'Z',1(a0)
	bne.s	.fail2
	cmp.b	#'G',2(a0)
	bne.s	.fail2

	# Get output buffer size
	moveq	#3,d0
	bsr	_LZG_GetUINT32
	move.l	(sp)+,d0
	rts

.fail2:	moveq	#0,d1
	move.l	(sp)+,d0
	rts


