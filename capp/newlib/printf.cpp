/*
	File:
		printf.c
	Notes:
		Small replacement for printf() functionality
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "printf.hpp"
#include "../sbc.hpp"
#include "../L1_Peripheral/uart.hpp"
#include "../debug.h"


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

void hexout(uint8_t digit);
void printhexb(uint8_t value);
void printhexw(uint16_t value);
void printhexl(uint32_t value);
void printd_padding(uint32_t value, uint8_t padding_width, uint8_t padding_type);
void printd(uint32_t value);

extern "C" {

int puts(const char *msg)
{
	int len = strlen(msg);
	uart.write((const uint8_t *)msg, len);
	return len;
}

} /* extern */

#include <stdarg.h>

int printf(const char *fmt, ...) 
{

	const char *ptr = (const char *)fmt;
//	uint32_t *parameters = &((uint32_t *)get_bp())[PARAMETER_BASE];
	uint32_t parameter;
	char ch;
	uint8_t state = STATE_PARSING;

	char width_fmt[8];
	int width_fmt_index = 0;
	bool width_leading = false;
	int width;

	va_list ap;
	va_start(ap, fmt);

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
						parameter = va_arg(ap, uint32_t);
						state = STATE_FORMATTING;
						width_leading = false;
						width = 0;
						width_fmt_index = 0;
						break;

					default:
						uart.write(ch);						
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
								printd_padding(parameter, width, !width_leading);
							}
							else
							{
								printd(parameter);
							}
							break;

						case 'x':
						case 'X':
							switch(width)
							{
								case 2:
									printhexb(parameter);
									break;
								case 4:
									printhexw(parameter);
									break;
								case 8:
									printhexl(parameter);
									break;
								default:									
									printhexl(parameter);
									break;
							}
							break;

						case 'c':
							uart.write(parameter);
							break;

						case 's':
							puts((char *)parameter);
							break;
					}
				}
				else 
				{
					switch(ch)
					{
						case '0':
						case '1': 
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':						
//						case '0' ... '9':
							width_fmt[width_fmt_index++] = ch;
							break;
			
						case '%':
							state = STATE_PARSING;
							uart.write('%');						
							break;

						default:
							puts("ERROR: Unknown format character [");
							printhexl(ch);
							puts(", ");
							printd(ch);
							puts(", ");
							uart.write(ch);
							puts("]\n");
							puts("ERROR: Unknown printf format.\n");
							trigger_hard_fault();
							break;
					} /* end switch ch */
				} /* end else */
				break; /* end case state */
		} /* end switch */
	} while (ch != 0);

	va_end(ap);

	return 0;
}





void hexout(uint8_t digit)
{
	uart.write("0123456789ABCDEF"[digit & 0x0F]);
}

void printhexb(uint8_t value)
{
	hexout((value >> 4) & 0x0F);
	hexout((value >> 0) & 0x0F);
}

void printhexw(uint16_t value)
{
	printhexb(	(value >> 8) & 0xFF);
	printhexb((value >> 0) & 0xFF);
}

void printhexl(uint32_t value)
{
	printhexw((value >> 16) & 0xFFFF);
	printhexw((value >> 0 ) & 0xFFFF);
}


/* Print 32-bit value as decimal with zero padding */
void printd_padding(uint32_t value, uint8_t padding_width, uint8_t padding_type)
{
	uint8_t stack[10];
	uint8_t index = 0; 

	/* Push decimal digits back-to-front */
	do {
		stack[index++] = (value % 10);
		value /= 10;		
	} while(value);

	
	/* Print leading zeros */
	if(padding_width)
	{
		for(int k = index; k < padding_width; k++)
		{
			if(padding_type == 0)
				hexout(0);
			else
			{
				uart.write(' ');
			}
		}
	}

	/* Pop digits and print them in order */
	while(index--)
	{
		hexout(stack[index]);
	}
}


/* Print 32-bit value as decimal */
void printd(uint32_t value)
{
	uint8_t stack[10];
	uint8_t index = 0; 

	/* Push decimal digits back-to-front */
	do {
		stack[index++] = (value % 10);
		value /= 10;		
	} while(value);

	/* Pop digits and print them in order */
	while(index--)
	{
		hexout(stack[index]);
	}
}










