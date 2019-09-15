

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sbc.h"


void *sbrk(int incr)
{
	static char *heap_end = NULL;
	char *prev_heap_end = NULL;

	/* If we haven't initialized hap_end to point to the end of the heap, do so now (highest address after end of loaded program)*/
	if(heap_end == NULL)
	{
		heap_end = &_end;
	}

	/* Take note of current heap end */
	prev_heap_end = heap_end;

	/* If adjustment of the heap collides with the stack, we're out of memory */
	if((uint32_t)(heap_end + incr) >= get_sp())
	{
		// collision
	}

	/* Adjust heap end */
	heap_end += incr;

	/* Return pointer to previous heap end */
	return (void *)prev_heap_end;
}



/*
	BP (A6) offset:
	0.l : Unallocated
	1.l : Return address of caller
	2.l : Format string
	3.l+: Parameters (reverse order)
*/

#define PARAMETER_BASE	3

	// states
	// 0 : processing string
	// 1 : processing format


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
							uart_printd(*parameters++);
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
							hard_fault();
							break;
					} /* end switch ch */
				} /* end else */
				break; /* end case state */
		} /* end switch */
	} while (ch != 0);
}

