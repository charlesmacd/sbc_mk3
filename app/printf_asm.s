
            .include "cpu_asm.inc"
#-------------------------------------------------------------------------------
# printf_asm.s
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Macros
#-------------------------------------------------------------------------------


#-------------------------------------------------------------------------------
# Print formatted string
#-------------------------------------------------------------------------------

# mode parse
# if % goto mode escape

# mode escape
# if % then literal %
# each character must match a valid escape sequence like
# %c %s %x %0x %04x %-s etc.
# for a parameter read longword off stack (all params long)
               
mode            =       d7
flags           =       d6
width           =       d5
digits          =       d4
flag_padding    =       0
flag_minus      =       1
mode_parse      =       0
mode_escape     =       1

                .macro  set_flag flag:req
                bset    #\flag, flags
                .endm

                .macro  is_match reg:req char:req label:req
                cmpi.b  #\char, \reg
                beq.s   \label
                .endm  

                .macro  get_format_char reg:req
                move.b  (a0)+, \reg
                tst.b   \reg
                beq.w   .eol
                .endm

                .global print_formatted
print_formatted:
                movem.l d0-d7/a0-a7, -(a7)
                
                # Point to parameter list
                lea     -38(sp), a6
                
                # Clear print state
                moveq   #mode_parse, mode
                moveq   #0, flags
                moveq   #0, width
                moveq   #0, digits
        .next_parse:
                get_format_char d0
                is_match d0, '%', .next_prefix

                # Print any non-escape character
                jsr     uart_putch_polling
                bra.s   .next_parse

        .next_prefix:
                get_format_char d0
                is_match d0, '-', .is_minus
                is_match d0, '0', .is_padding
                bra.w   .parse_escape
        .is_minus:
                set_flag flag_minus
                bra.w   .next_escape
        .is_padding:
                set_flag flag_padding
                bra.w   .next_escape
        
        .next_escape:
                get_format_char d0
        .parse_escape:

                # Check if character is ASCII '0'-'9'
                cmp.b   #'0', d0
                blt.s   .not_digit
                cmpi.b  #'9', d0
                bgt.s   .not_digit
        .is_digit:
                # Convert ASCII '0'-'9' to binary 0-9
                sub.b   #'0', d0
                add.l   d0, digits
                

        .not_digit:
                is_match d0, 'c', .parse_char
                is_match d0, 'd', .parse_digit
                is_match d0, 'x', .parse_hex
                is_match d0, 'X', .parse_hex
                is_match d0, 'p', .parse_pointer
                is_match d0, 's', .parse_string

                # Error, unknown character in escape sequence
                bra.w   .eol

        .parse_char:
                move.l  (a6)+, d0
                jsr     uart_putch_polling
                bra.w   .next_parse

        .parse_hex:
        .parse_digit:
                ror.w   #4, digits
                move.l  (a6)+, d0
                jsr     printhexl
                bra.w   .next_parse

      .parse_pointer:
                move.l  (a6)+, d0
                jsr     printhexl
                bra.w   .next_parse

        .parse_string:
                move.l  a0, -(sp)
                jsr     uart_puts_polling
                move.l  (sp)+, a0
                bra.w   .next_parse
        .eol:
                rts


is_digit:
                cmpi.b  #'0', d0
                blt.s   1f
                cmpi.b  #'9', d0
                bgt.s   1f
                ori.b   #F_CARRY, ccr
                rts
        1:      andi.b  #~F_CARRY, ccr                
                rts
#-------------------------------------------------------------------------------
# End
#-------------------------------------------------------------------------------

clrscr:
                rts

gotoxy:
                rts

                .global printhexd
printhexd:
                move.w  d0, -(sp)
                andi.w  #0x0F, d0
                move.b  .hextab(pc, d0.w), d0
                jsr     uart_putch_polling
                move.w  (sp)+, d0
                rts
        .hextab:
                .ascii "0123456789ABCDEF"                

                .global printhexb
printhexb:
                move.w  d0, -(sp)
                rol.b   #4, d0
                bsr.s   printhexd
                rol.b   #4, d0
                bsr.s   printhexd
                move.w  (sp)+, d0
                rts                

                .global printhexw
printhexw:
                move.w  d0, -(sp)
                .rept   4
                rol.w   #4, d0
                bsr.s   printhexd
                .endr
                move.w  (sp)+, d0
                rts                

                .global printhexl
printhexl:
                move.l  d0, -(sp)
                .rept   8
                rol.l   #4, d0
                bsr.s   printhexd
                .endr
                move.l  (sp)+, d0
                rts              

                .global newline
newline:
                moveq   #'\n', d0
                jsr     uart_putch_polling
                rts
