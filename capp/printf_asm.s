
            .include "L0_Platform/cpu_asm.inc"
#-------------------------------------------------------------------------------
# printf_asm.s
#-------------------------------------------------------------------------------

PRINTF_BUFFER_SIZE      =       128

                .section .KERNELVARS
                .global  __printf_buffer
                .global  __printf_buffer_end
__printf_buffer:
                .space PRINTF_BUFFER_SIZE
__printf_buffer_end:       
__printf_save:
                .space 4*32


                .global  __kernel_page_buffer
__kernel_page_buffer:
                .space  256
__kernel_page_buffer_end:       
                .section .text

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





param_base      =       a5
param_ptr       =       a6
input_ptr       =       a2
output_buffer   =       a1               
mode            =       d7

flags           =       d6
width           =       d5
digits          =       d4
temp1           =       d1
temp0           =       d0

flag_padding    =       0
flag_minus      =       1
mode_parse      =       0
mode_escape     =       1

                .macro  set_flag flag:req
                bset    #\flag, flags
                .endm

                .macro  is_match reg:req char:req label:req
                cmpi.b  #\char, \reg
                beq.w   \label
                .endm  

                .macro  get_format_char reg:req
                move.b  (input_ptr)+, \reg
                tst.b   \reg
                beq.w   .eol
                .endm

                .macro  write_output_char reg:req
#                move.b  \reg, (output_buffer)+
                jsr     uart_putch_polling
                .endm

                .macro  get_stacked_parameter reg:req
                cmpa.l  a5, a6
                beq.w   .error_parameter_miscount
                move.l  -(a6), \reg
                .endm


# 4 pea nargs.l
# 4 push a0.l
# 4 jsr func (push return address)
# 64 push all res



                .global print_formatted

print_formatted:

                # Save registers
                movem.l d0-d7/a0-a7, -(sp)

                # Point A6 to start of pushed parameters
                lea    0x44(sp), param_ptr

                # Get address of format string
                move.l  (param_ptr)+, input_ptr

                # Get number of arguments
                move.l  (param_ptr)+, d0

                # Convert argument count to byte count
                lsl.w   #2, d0

                # Point to start of pushed parameters
                move.l  param_ptr, param_base
                lea     (param_ptr, d0.w), param_ptr

                # Clear print state
                moveq   #mode_parse, mode
                moveq   #0, flags
                moveq   #0, width
                moveq   #0, digits
                lea     __printf_buffer, output_buffer

        .next_parse:

                # Check for start of escape sequence
                get_format_char d0
                is_match d0, '%', .next_prefix

                # Print any non-escape character
                write_output_char d0
                bra.s   .next_parse

        .next_prefix:

                # Check for sign or padding
                get_format_char d0
                is_match d0, '-', .is_minus
                is_match d0, '0', .is_padding
                bra.w   .parse_escape

        .is_minus:
                # Sequence was "%-"
                set_flag flag_minus
                bra.w   .next_escape

        .is_padding:
                # Sequence was "%0"
                set_flag flag_padding
                bra.w   .next_escape

                nop

        .next_escape:

                # Get next character in escape sequence
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

                # Isolate digit
                moveq   #0x0F, d1
                and.l   d0, d1

                # Shift digits up by one decimal place
                mulu.w  #10, digits

                # Add new digit in
                add.l   d1, digits
                bra.s   .next_escape

        .not_digit:

                # Check for escape sequence terminating character
                is_match d0, 'c', .parse_char
                is_match d0, 'd', .parse_digit
                is_match d0, 'x', .parse_hex
                is_match d0, 'X', .parse_hex
                is_match d0, 'p', .parse_pointer
                is_match d0, 's', .parse_string
                bra.w   .error_unknown_escape

        .parse_char:

                # Output character 
                get_stacked_parameter d0
                write_output_char (input_ptr)+
                bra.w   .next_parse

        .hextable:  
                .ascii "0123456789ABCDEF"
                .align  2

        .parse_hex:
                # Determine width of hex value

                cmpi.b  #8, digits 
                beq.s   .print_long
                cmpi.b  #4, digits
                beq.s   .print_word
                cmpi.b  #2, digits
                beq.s   .print_byte

        .print_long:
                get_stacked_parameter d0
                jsr     printhexl
                bra.w   .next_parse

        .print_word: 
                get_stacked_parameter d0            
                jsr     printhexw
                bra.w   .next_parse

        .print_byte:                
                get_stacked_parameter d0
                jsr     printhexb
                bra.w   .next_parse

        .parse_digit:
                get_stacked_parameter d0
                andi.l  #0x0F, d0
                ori.b   #'0', d0
                jsr     uart_putch_polling
                bra.w   .next_parse

        .parse_pointer:
                get_stacked_parameter d0
                jsr     printhexl
                bra.w   .next_parse

        .parse_string:
                get_stacked_parameter a0
                jsr     uart_puts_polling
                bra.w   .next_parse

        .error_unknown_escape:
                # Error, unknown character in escape sequence
                # Set error flag
                bra.w   .eol

        .error_parameter_miscount:
                # set error flag
                bra.s   .eol
                nop
        .eol:
                # print buffer
                movem.l (sp)+, d0-d7/a0-a7
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

# convert to ascii without lookup?



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
                move.l  d0, -(sp)
                moveq   #'\n', d0
                jsr     uart_putch_polling
                move.l  (sp)+, d0
                rts


# Convert nibble in D0 to ASCII values 0-9/A-F
# Input is nibble in D0
# Output is ASCII character in D0
bin2ascii:
                move.w  d1, -(sp)

                # Isolate nibble
                andi.b  #0x0F, d0

                # Clear extend flag for addition
                andi.b  #~F_EXTEND, ccr

                # Add 0x90 (BCD) to nibble to convert to:
                # If nibble is 00-09, output is 90-99 with XF=0
                # If nibble is 0A-0F, output is 00-05 with XF=1
                move.b  #0x90, d1
                abcd    d1, d0

                # Add 0x40 (BCD) and extend flag to sum to convert to:
                # If sum is 90-99, output is 30-39 with XF=0 ('0'-'9')
                # If sum is 00-05, output is 41-46 with XF=1 ('A'-'F')
                move.b  #0x40, d1
                abcd    d1, d0

                move.w  (sp)+, d1
                rts
