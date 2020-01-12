

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
using namespace std;

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


//////////////////////////////////////////////?/////////////?//////////////////////

#define HISTORY_MAX_SIZE		128

class HistoryBuffer
{
public:
	uint8_t capacity;
	uint8_t index_mask;
	uint8_t write_index;
	uint8_t read_index;
    uint8_t count;
	uint8_t buffer[RINGBUF_MAX_SIZE];
   
	void print_buf(void)
	{
		for(int i = 0; i < capacity; i++)
		{
//			buffer[i] = 0xbc;
			printf("[%02X]", buffer[i]);
		}
		printf("\n");
	}


    /* Intialize ring buffer */
    bool initialize(uint32_t buffer_capacity)
    {
        /* Validate capacity is a non-zero power-of-two */
        if(buffer_capacity == 0 || __builtin_popcount(buffer_capacity) != 1)
        {
            return false;
        }
		


        capacity = buffer_capacity;
        index_mask = (capacity - 1);
        write_index = 0;
        read_index = 0;
        count = 0;

        return true;
    }

    bool peek(uint8_t offset, uint8_t &value)
    {
        value = buffer[(write_index - offset) & index_mask];
		if(offset > count)
		{
			return false;
		}
        return true;
    }

	void consume(uint8_t amount)
	{
		count -= amount;

		// clamp
		if(!amount)
		{
			count = 0;
		}
			
	}

    /* Insert byte into ring buffer */
    void insert(uint8_t data)
    {
        buffer[write_index++ & index_mask] = data;
        ++count;
    }


    /* Test if ring buffer is empty */
    int empty(void)
    {
        return (count == 0);
    }
};



//////////////////////////////////

line_editor_history_t line_editor_history;


class LineEditor
{
	Uart *uart;
	HistoryBuffer history;
public:
	size_t edit(Uart &uart, char *buf, int buf_size, void *fileno, int (*escape_handler)(uint8_t code, line_editor_state_t *state));
	bool evaluate_escape(char ch);
};

#if 0

typedef struct
{
	char buffer[80];
	int index;
	char esc_buffer[10];
	int esc_index;
	uint8_t esc_flag;
} line_editor_state_t;
#endif

// keep last n items in history 
// shift out bytes as we insert new ones
// length of sft is equal to longest sequence we want to detect
// how to implement?
// just ring buffer that doesn't overflow/underflow, kinda
// well for inserts, we just insert and wrap
// how to know # of readable bytes?
// on each insert bump count (# of inserted)
// we can read from write index, backwards 




//{"\x1B\x5B\x41", 		 3, VK_UP},
	
bool LineEditor::evaluate_escape(char ch)
{
	printf("\nINIT: ");
	history.print_buf();


	history.insert(ch);

	if(history.count >= 3)
	{
		for(int i = 0; i < 3; i++)
		{
			uint8_t value;
			history.peek(i, value);
			printf("Index:%02X | Value:%02X\n", i, value);
		}
	}


	return true;
}	

size_t LineEditor::edit(Uart &uart, char *buf, int buf_size, void *fileno, int (*escape_handler)(uint8_t code, line_editor_state_t *state))
{
	const bool local_echo = true;
	const int esc_buffer_size = 10;

	line_editor_state_t state;

	/* Assume not escaping by default */
	memset(&state, 0, sizeof(line_editor_state_t));

	bool insert = true;

	history.initialize(8);

	while(1)
	{
		insert = true;

		/* Wait until there is data in the UART ring buffer (populated by UART ISR) */
		char ch = uart.read_blocking();

		/* Newline (handle regardless of escaping) */
		if(ch == ASCII_CR)
		{
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
			/* Not escaping */
			switch(ch)
			{
				case ASCII_ESC: /* Found escape character, start escape sequence */
//					printf("[%02X](%08X)", ch, uart.rx_ringbuf.count);					
					break;

				default:
					break;
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

			if(local_echo)
			{
				uart.write(ch);
			}
		}
	}
	
	return 0;
}


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



LineEditor ed;


int process_command_prompt(cli_cmd_t *commands, const char *prompt)
{
	const int line_width = 80;
	char buf[line_width];
	size_t size = 0;

	/* Display prompt to user */
	printf("%s", prompt);


	/* Block until user input available */
	size = ed.edit(
		uart,
		buf, 
		sizeof(buf), 
		NULL, 
		esc_handler);

	/* Process input */
	if(size)
	{
		return process_cli(buf, size, commands);
	}

	return 0;
}



