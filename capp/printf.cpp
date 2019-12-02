/*
	File:
		printf.c
	Notes:
		Small replacement for printf() functionality
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "printf.hpp"
#include "sbc.hpp"
#include "peripheral/uart.hpp"



/*
	BP (A6) offset:
	0.l : Unallocated
	1.l : Return address of caller
	2.l : Format string
	3.l+: Parameters (reverse order)
*/
#define PARAMETER_BASE		3
#define STATE_PARSING		0
#define STATE_FORMATTING	1


int printf(const char *fmt, ...) 
{
	char *ptr = (char *)fmt;
	uint32_t *parameters = &((uint32_t *)get_bp())[PARAMETER_BASE];
	char ch;
	uint8_t state = STATE_PARSING;

	char width_fmt[8];
	char width_fmt_index = 0;
	bool width_leading = false;
	int width;

	do
	{
		/* Get character from format string, may contain literal text or formatting characters */
		ch = *ptr++;

		if(ch == 0)
			break;

		switch(state)
		{
			case STATE_PARSING: /* Regular string */
				switch(ch)
				{
					case '%':
						state = STATE_FORMATTING;
						width_leading = false;
						width = 0;
						width_fmt_index = 0;
						break;

					default:
						uart_putch(ch);						
						break;
				}
				break;

			case STATE_FORMATTING: /* Format specifier */

				if(strchr("dxXcs", ch))
				{
					state = STATE_PARSING;

					if(width_fmt_index)
					{
						if(width_fmt[0] == '0')
							width_leading = true;
						width_fmt[width_fmt_index] = 0;
						width = strtol(width_fmt, NULL, 10);
					}

					switch(ch)
					{
						case 'd':
							if(width)
							{
								uart_printd_padding(*parameters++, width, !width_leading);
							}
							else
							{
								uart_printd(*parameters++);
							}
							break;

						case 'x':
						case 'X':
							switch(width)
							{
								case 2:
									uart_printhexb(*parameters++);
									break;
								case 4:
									uart_printhexw(*parameters++);
									break;
								case 8:
									uart_printhexl(*parameters++);
									break;
								default:									
									uart_printhexl(*parameters++);
									break;
							}
							break;

						case 'c':
							uart_putch(*parameters++);
							break;

						case 's':
							uart_puts((char *)*parameters++);
							break;
					}
				}
				else 
				{
					switch(ch)
					{
						case '0' ... '9':
							width_fmt[width_fmt_index++] = ch;
							break;
			
						case '%':
							state = STATE_PARSING;
							uart_putch('%');						
							break;

						default:
							uart_puts("ERROR: Unknown format character [");
							uart_printhexl(ch);
							uart_puts(", ");
							uart_printd(ch);
							uart_puts(", ");
							uart_putch(ch);
							uart_puts("]\n");
							uart_puts("ERROR: Unknown printf format.\n");
							trigger_hard_fault();
							break;
					} /* end switch ch */
				} /* end else */
				break; /* end case state */
		} /* end switch */
	} while (ch != 0);
}

