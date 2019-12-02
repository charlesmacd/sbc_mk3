#-------------------------------------------------------------------------------
# Macros
#-------------------------------------------------------------------------------

# Save registers
		.macro	push	reglist
		movem.l	\reglist, -(a7)
		.endm

# Restore registers
		.macro	pop	reglist
		movem.l	(a7)+, \reglist
		.endm

# Clear carry flag
		.macro	clc
		andi.b	#~1, ccr
		.endm

# Set carry flag
		.macro	sec
		ori.b	#1, ccr
		.endm

# Disable interrupts
		.macro	DISABLE_INTERRUPTS
		ori.w	#0x0700, sr
		.endm

# Enable all interrupts
		.macro	ENABLE_INTERRUPTS
		andi.w	#0xF8FF, sr
		.endm

# Save interrupt priority level and disable all interrupts
		.macro	ENTER_CRITICAL_SECTION
		move.w	sr, -(a7)
		sei
		.endm

# Restore all interrupts
		.macro	EXIT_CRITICAL_SECTION
		move.w	(a7)+, sr
		.endm

#-------------------------------------------------------------------------------
# End
#-------------------------------------------------------------------------------

