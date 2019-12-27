#-------------------------------------------------------------------------------
# printf_asm.inc
#-------------------------------------------------------------------------------

nargs           =       0
parsed          =       0

# Macro to count number of arguments

                .macro count_args first, args:vararg
                    nargs = 0
                    count_args_helper \first, \args
                .endm

                .macro count_args_helper first, args:vararg
                    .ifnb \first
                        count_args_helper \args
                        nargs = nargs + 1
                    .endif
                .endm

                # Match a token to symbols "x0-x7" where x is the prefix character
                # Matching tokens are pushed to the stack as longwords
                .macro pushl_xn prefix:req parameter:req
                     # Bail if we already parsed this parameter
                    .if parsed == 1
                        .mexit
                    .endif
                    # Compare parameter to x0-x7
                    .irp number,0,1,2,3,4,5,6,7
                        .if \parameter == \prefix\number 
                            # Stack matching parameter
                            move.l \prefix\number, -(sp)
                            # Flag parameter as parsed
                            parsed = 1
                            .mexit
                        .endif
                    .endr
                .endm


# Print string with variable arguments

                .macro printf format:req tokens:vararg

                # Parse all variable arguments
                # Operates as 'foreach parameter in tokens'
                .irp    parameter, \tokens

                # Flag current parameter as not being parsed
                parsed = 0

                # Check if parameter matches any of a0-a7
                # This will set parse if there was a match
                pushl_xn a, \parameter

                # Check if parameter matches any of d0-d7
                # This will set parse if there was a match
                pushl_xn d, \parameter

                # Check if parameter matches any of sr
                # This will set parse if there was a match
                # NOTE: Pushes zero to pad 16-bit SR to 32-bit quanitty
                .if parsed == 0
                    .if \parameter == sr
                        subq.l #2, sp
                        move.w sr, -(sp)
                        parsed = 1
                    .endif
                .endif

                # Check if parameter matches any of sp
                # This will set parse if there was a match
                .if parsed == 0
                    .if \parameter == sp
                        move.l sp, -(sp)
                        parsed = 1
                    .endif
                .endif

                # Push any remaining token as data,
                # assuming it is a numeric literal or some other symbol
                .if     parsed == 0
                pea    (\parameter).l
                parsed = 1
                .endif

                .endr # /* irp tokens */

                # Save registers
                move.l  a0, -(sp)

                # Get address of format string in A0
                lea     (pc, data\@), a0

                # Print formatted string
                jsr     print_formatted

                # Skip past format string
                bra.s   skip\@

                # Format string
data\@:         .asciz "\format"
                .align  2

                # Restore registers
skip\@:         movem.l (sp)+, a0
                .endm

#-------------------------------------------------------------------------------
# End
#-------------------------------------------------------------------------------
