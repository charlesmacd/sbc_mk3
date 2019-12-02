
                .include "macros.inc"
                .include "cpu_asm.inc"
                .include "hw_defs.inc"

# Terminal (UART) printing functions
SYSCALL_CLRSCR          =   0x00
SYSCALL_NEWLINE         =   0x01
SYSCALL_PUTS            =   0x02
SYSCALL_PUTCH           =   0x03
SYSCALL_PRINTF          =   0x04
SYSCALL_GOTOXY          =   0x05
SYSCALL_TEXTATTR        =   0x06

# D0 = Function number
_trap_syscall:
        movem.l d0/a0, -(sp)
        andi.w  #0x000F, d0
        lsl.w   #2, d0
        move.l  syscall2_dispatch(pc, d0.w), a0
        pea     1f
        jmp     (a0)
1:      movem.l (sp)+, d0/a0
        rte

syscall2_dispatch:
        dc.l    __stub
        dc.l    __stub
        dc.l    __stub
        dc.l    __stub
        dc.l    __stub
        dc.l    __stub
        dc.l    __stub
        dc.l    __stub
__stub:
        rts                


__isr_syscall:
        # Save registers
        movem.l a0/d0, -(sp)

        # Get stacked PC
        move.l  10(sp), a0

        # Read lower 4 bits of opcode at PC
        moveq   #0x0F, d0
        and.w   (a0), d0

        # Convert to longword table index
        add.w   d0, d0
        add.w   d0, d0

        # Jump to vector at table index
        move.l  syscall_dispatch(pc, d0.w), a0
        pea     syscall_exit
        jmp     (a0)

syscall_exit:
        movem.l (sp)+, a0/d0
        rte

syscall_dispatch:
    #00
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
    #08        
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
        dc.l    syscall_unused
    #10

syscall_unused:
        rts
