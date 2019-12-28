#-------------------------------------------------------------------------------
# printf_asm.inc
#-------------------------------------------------------------------------------


nargs           =       0
parsed          =       0


# Macro to count number of arguments

                # Match a token to symbols "x0-x7" where x is the prefix character
                # Matching tokens are pushed to the stack as longwords
                .macro push_register prefix:req parameter:req
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

                # Initially assume there are no parameters
                nargs   = 0

                # If parameters exist, process them
                .ifnb   \tokens

                # Count number of parameters present
                .irp    parameter, \tokens
                nargs = nargs + 1
                .endr /* irp tokens */

                # Parse all parameters
                .irp    parameter, \tokens

                # Flag current parameter as not being parsed
                parsed = 0

                # Check if parameter matches any of a0-a7
                # This will set parse if there was a match
                push_register a, \parameter

                # Check if parameter matches any of d0-d7
                # This will set parse if there was a match
                push_register d, \parameter

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

                # Push any remaining parameter as data,
                # assuming it is a numeric literal or some other symbol
                # indirection here?
                .if     parsed == 0
                pea    (\parameter).l
                parsed = 1
                .endif

                .endr # /* end irp tokens */

                .endif /* end ifnb tokens */

                # Push number of arguments found as longword
                pea     (nargs).l

                # Get address of format string in A0
                # pea this?
                pea     (pc, data\@)

                # Print formatted string
                jsr     print_formatted

                # Skip past format string
                bra.s   skip\@

                # Format string
data\@:         .asciz "\format"
                .align  2

skip\@:         
                # Adjust stack for pushed parameters (nargs), argument count (1), string address (1)
                adda.l  #(4*(nargs+2)), sp

                # Caller code resumes here
                .endm



#-------------------------------------------------------------------------------
# End
#-------------------------------------------------------------------------------
