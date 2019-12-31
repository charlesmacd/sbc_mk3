
#include <stdio.h> // sprintf
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "../sbc.hpp"
#include "../L1_Peripheral/uart.hpp"
#include "../L1_Peripheral/timer.hpp"
#include "../L3_Application/cli.hpp"
#include "../L3_Application/app.hpp"
#include "../L3_Application/term_diag.hpp"
#include "../newlib/mem_heap.hpp"
#include "term_main.hpp"
#include "../debug.h"


static int cmd_ints(int argc, char *argv[]);
static int cmd_list(int argc, char *argv[]);
static int cmd_exit(int argc, char *argv[]);

static cli_cmd_t terminal_cmds[] = 
{
	{"?",       cmd_list},
	{"help",    cmd_list},
	{"ints",    cmd_ints},
    {"exit",    cmd_exit},
	{NULL,      NULL},
};

/*----------------------------------------------------------------------------*/

int cmd_ints(int argc, char *argv[])
{
    printf("Starting interrupt test.\n");
    printf("Test complete.\n");
    return -1;
}

/*----------------------------------------------------------------------------*/

int cmd_exit(int argc, char *argv[])
{
    return -1;
}

/*----------------------------------------------------------------------------*/

int cmd_list(int argc, char *argv[])
{	
	cli_cmd_t *cli_cmd_tab = terminal_cmds;

	printf("Available commands:\n");
	for(int i = 1; cli_cmd_tab[i].name != NULL; i++)
	{
		printf("- %s\n", cli_cmd_tab[i].name);
	}
	return 0;
}

/*----------------------------------------------------------------------------*/

int cmd_diag(int argc, char *argv[])
{
    bool running = true;
    while(running)
    {
        int status = process_command_prompt(terminal_cmds, "diag:\\>");
        if(status == -1)
        {
            running = false;
            break;
        }
        WAIT_IRQ_ANY();
    }
	return 0;
}
