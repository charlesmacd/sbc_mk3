
#ifndef _CLI_H_
#define _CLI_H_

#define ANSI_HOME		"\033[H"
#define ANSI_CLRSCR		"\033[2J"
#define CMD_HISTORY_MAX		8
#define CMD_LENGTH_MAX		80



#define MODE_NORMAL		0
#define MODE_ESCAPE1	1	/* After getting 1B */
#define MODE_ESCAPE2	2	/* After getting 5B */
#define MODE_ESCAPE3	3
#define MODE_ESCAPE		4

#define ASCII_ESC		0x1B
#define ASCII_CR		0x0D
#define ASCII_LF		0x0A
#define ASCII_DEL		0x7F

#define MAX_HISTORY		10

enum virtual_key {
	VK_F1	=	0,
	VK_F2,
	VK_F3,
	VK_F4,
	VK_F5,
	VK_F6,
	VK_F7,
	VK_F8,
	VK_F9,
	VK_F10,
	VK_F11,
	VK_F12,
	VK_PGUP,
	VK_PGDN,
	VK_UP,
	VK_DOWN,
	VK_LEFT,
	VK_RIGHT,
	VK_INSERT,
	VK_DELETE,
	VK_HOME,
	VK_END,
	MAX_VK
};

typedef struct
{
	const char *seq;
	uint8_t length;
	uint8_t keycode;
} esc_seq_t;


typedef struct
{
	char buffer[80];
	int index;
	char esc_buffer[10];
	int esc_index;
	uint8_t esc_flag;
} line_editor_state_t;

typedef struct
{
	char buffer[80];
} line_editor_history_item_t;

typedef struct
{
	line_editor_history_item_t items[MAX_HISTORY];
	int index;
} line_editor_history_t;

typedef struct 
{
	const char *name;
	int (*func)(int argc, char *argv[]);
} cli_cmd_t;

/* Function prototypes */
int process_command_prompt(cli_cmd_t *commands, const char *prompt);


#endif /* _CLI_H_ */