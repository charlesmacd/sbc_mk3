

// process cli and line editor use char, should be byte
/*
	File:
		cli.c
	Author:
		Charles MacDonald
	Notes:
		None
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "../sbc.hpp"
#include "../L1_Peripheral/uart.hpp"


/* Function prototypes */
extern int sprintf(char *str, const char *format, ...);


static const esc_seq_t esc_seq_tab[] =
{
	{"\x1B\x5B\x41", 		 3, VK_UP},
	{"\x1B\x5B\x42", 		 3, VK_DOWN},
	{"\x1B\x5B\x44", 		 3, VK_LEFT},
	{"\x1B\x5B\x43", 		 3, VK_RIGHT},
	{"\x1B\x5B\x31\x7E", 	 4, VK_INSERT},
	{"\x1B\x5B\x32\x7E", 	 4, VK_HOME},
	{"\x1B\x5B\x33\x7E", 	 4, VK_PGUP},
	{"\x1B\x5B\x34\x7E", 	 4, VK_DELETE},
	{"\x1B\x5B\x35\x7E", 	 4, VK_END},
	{"\x1B\x5B\x36\x7E", 	 4, VK_PGDN},
	{"\x1B\x5B\x31\x31\x7E", 5, VK_F1},
	{"\x1B\x5B\x31\x32\x7E", 5, VK_F2},
	{"\x1B\x5B\x31\x33\x7E", 5, VK_F3},
	{"\x1B\x5B\x31\x34\x7E", 5, VK_F4},
	{"\x1B\x5B\x31\x35\x7E", 5, VK_F5},
	{"\x1B\x5B\x31\x37\x7E", 5, VK_F6},
	{"\x1B\x5B\x31\x38\x7E", 5, VK_F7},
	{"\x1B\x5B\x31\x39\x7E", 5, VK_F8},
	{"\x1B\x5B\x32\x30\x7E", 5, VK_F9},
	{"\x1B\x5B\x32\x31\x7E", 5, VK_F10},
	{"\x1B\x5B\x32\x33\x7E", 5, VK_F11},
	{"\x1B\x5B\x32\x34\x7E", 5, VK_F12},	
	{NULL, 0, 0}
};

line_editor_history_t line_editor_history;


/*
	history buffer
	on successful command entry, add to buffer and bump index
	need to store string, mainly for argv
*/

void line_editor_reset(void)
{
	memset(&line_editor_history, 0, sizeof(line_editor_history));
	line_editor_history.index = 0;
}



size_t line_editor(char *buf, int buf_size, void *fileno, int (*escape_handler)(uint8_t code, line_editor_state_t *state))
{
	const int esc_buffer_size = 10;

	line_editor_state_t state;

	/* Assume not escaping by default */
	memset(&state, 0, sizeof(line_editor_state_t));

	bool insert = true;

	while(1)
	{
		insert = true;

		/* Wait until there is data in the UART ring buffer (populated by UART ISR) */
		char ch = uart.read_blocking();

		/* Newline (handle regardless of escaping) */
		if(ch == ASCII_CR)
		{
			if(state.esc_flag)
			{
				/* We're escaping */
				// FIXME for 2 or more ESCs in sequence
				if(state.esc_index == 1)
				{
					/* Consume ESC and do nothing */
				}
				else
				{
					// we have an escape sequence being processed that was not finished yet or is invalid
					printf("*** Error, found newline before escape sequence was terminated.\n");
					for(int i = 0; i < state.esc_index; i++)
					{
						printf("ESC #%d : [%02X]\n", i, state.esc_buffer[i]);
					}
				}
			}

			/* Echo CR,LF in response to each CR recieved */
			uart.write(ASCII_CR);
			uart.write(ASCII_LF);

			/* Terminate line */
			state.buffer[state.index] = 0;
			strcpy((char *)buf, state.buffer);
			return state.index;
		}
		else
		if(ch == ASCII_DEL)
		{
			/* Delete previous character if present */
			if(state.index)
			{
				state.buffer[--state.index] = 0;
				uart.write(ASCII_DEL);
			}

			/* Don't insert this character into the buffer */
			insert = false;
		}
		else
		{
			if(state.esc_flag)
			{
				/* Insert character to escape buffer */
				state.esc_buffer[state.esc_index++] = ch;
				if(state.esc_index >= esc_buffer_size)
					state.esc_index = esc_buffer_size - 1;

				/* See if any escape sequences match what's in the escape buffer */
				/* FIXME Find a faster way to do this */
				bool searching = true;
				for(int i = 0; i < MAX_VK && searching; i++)
				{
					if(state.esc_index == esc_seq_tab[i].length 
						&& memcmp(state.esc_buffer, esc_seq_tab[i].seq, state.esc_index) == 0)
					{
						if(escape_handler != NULL)
						{
							escape_handler(esc_seq_tab[i].keycode, &state);
							searching = false;
						}

						state.esc_flag = 0;
						state.esc_index = 0;

						/* Don't insert this character into the buffer */
						insert = false;
					}
				}
			}
			else
			{
				/* Not escaping */
				switch(ch)
				{
					case ASCII_ESC: /* Found escape character, start escape sequence */
						state.esc_flag = 1;
						state.esc_index = 0;
						memset(state.esc_buffer, 0, esc_buffer_size);

						/* Insert character into escape buffer */
						state.esc_buffer[state.esc_index++] = ch;
						if(state.esc_index >= esc_buffer_size)
							state.esc_index = esc_buffer_size - 1;
						break;

					default:
						break;
				}			
			}
		}

		/* Insert character into buffer */
		if(state.esc_flag == 0 && insert)
		{
			state.buffer[state.index++] = ch;
			if(state.index >= buf_size)
			{
				state.index = buf_size - 1;
			}

			uart.write(ch);
		}
	}
}



int esc_handler(uint8_t code, line_editor_state_t *state)
{
	switch(code)
	{
		case VK_UP:
			break;
		case VK_DOWN:
			break;
		case VK_LEFT:
			break;
		case VK_RIGHT:
			break;

		default:
			printf("<V:%d>", code);
			return 0;
	}
	return 0;
}






int process_cli(char *buf, int size, cli_cmd_t *cli_cmd)
{
	/* Maximum number of arguments to a command */
	const int max_tokens = 8;

	bool found = false;
	char *tokens[max_tokens];
	int token_count = 0;

	char *tmp = strtok((char *)	buf, " ");
	while(tmp)
	{
		if(token_count < max_tokens)
		{
			tokens[token_count++] = tmp;
		}
		else
		{
			printf("Error: Too many tokens specified.\n");
			return 0;
		}
		tmp = strtok(NULL, " ");
	}

	/* No tokens found */
	if(token_count == 0)
	{
		return 0;
	}

	/* See if any commands match the first token */
	for(int i = 0; cli_cmd[i].name != NULL && !found; i++)
	{
		if(strcmp(tokens[0], cli_cmd[i].name) == 0)
		{
			found = true;
			/*
				insert into history
				memcpy(line_editor_history.items[line_editor_history.index], buf, strlen(buf));
				line_editor_history.index = (line_editor_history_index + 1) & 7;

				command return value should indicate if it gets added or not
			*/
			return cli_cmd[i].func(token_count, tokens);
		}
	}

	if(found == false)
	{
		printf("Error: Unrecognized input `%s'.\n", buf, size);
	}

	return 0;
}





void process_command_prompt(cli_cmd_t *commands)
{
	const int line_width = 80;
	const char *prompt = "os>";
	char buf[line_width];
	size_t size = 0;

	/* Display prompt to user */
	printf("%s", prompt);

	/* Block until user input available */
	size = line_editor(
		buf, 
		sizeof(buf), 
		NULL, 
		esc_handler);

	/* Process input */
	if(size)
	{
		process_cli(buf, size, commands);
	}
}




