/*
	File: term_diag.cpp
*/

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
#include "../L1_Peripheral/sbc_pio.hpp"
#include "../newlib/mem_heap.hpp"
#include "term_main.hpp"
#include "../debug.h"


static int cmd_list(int argc, char *argv[]);
static int cmd_timer(int argc, char *argv[]);
static int cmd_exit(int argc, char *argv[]);
static int cmd_pioram(int argc, char *argv[]);

static cli_cmd_t terminal_cmds[] = 
{
	{"?",       cmd_list},
	{"help",    cmd_list},
	{"timer",   cmd_timer},
    {"exit",    cmd_exit},
	{"pioram", 	cmd_pioram},
	{NULL,      NULL},
};

/*----------------------------------------------------------------------------*/

#if 1
int cmd_pioram(int argc, char *argv[])
{
	bool error = false;

	printf("Testing PIO RAM ...\n");

	const uint8_t pattern[] = {0x00, 0x55, 0xAA, 0xFF, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

	SBCPIO pio;
	pio.initialize();

	for(int k = 0; k < 0x18; k++)
	{
		pio.ram.write(k, "\xDE\xAD\xBE\xEF"[k&3]);
	}

	#if 0
	for(int loop = 0; loop < sizeof(pattern); loop++)
	{
		printf("Testing pattern: %02X (%d of %d)\n", pattern[loop], 1+loop, sizeof(pattern));

		for(int i = 0; i < pio.ram.size(); i++)
		{
			pio.ram.write(i, pattern[loop]);
		}

		for(int i = 0; i < pio.ram.size(); i++)
		{
			uint8_t expected = pattern[loop];
			uint8_t temp = pio.ram.read(i);
			if(temp != expected)
			{
				error = true;
				printf("Data mismatch, got %02X, expected %02X\n", temp, expected);
				break;
			}
		}
	}
	#endif

	printf("Test complete, RAM status is %s.\n", error ? "NG" : "OK");
	return 0;
}
#endif

int cmd_timer(int argc, char *argv[])
{
	bool running = true;

	printf("Starting timer test ...\n");
	printf("SR = %04X\n", get_sr());

	__systick_flag = 0;
	__interval_1us_flag = 0;
	__interval_1ms_flag = 0;

	uint32_t systick_count = 0;
	uint32_t interval_1ms_count = 0;
	uint32_t interval_1us_count = 0;
	uint32_t capture_1ms = 0xdeadbeef;
	uint32_t capture_1us = 0xdeadbeef;

	int interval_1ms = 500; 			// 500ms
	int interval_1us = 500 * 1000;		// 500,000us

	printf("Set 1ms timer to %d ticks.\n", interval_1ms);
	set_1ms_timer(interval_1ms);	
	
	printf("Set 1us timer to %d ticks.\n", interval_1us);
	set_1us_timer(interval_1us);	

	while(running)
	{
		if(uart.keypressed())
		{
			running = false;
			printf("Status: User requested exit.\n");
		}

		if(__interval_1ms_flag)
		{
			__interval_1ms_flag = 0;
			++interval_1ms_count;
			capture_1ms = systick_count;
			printf("Status: 1ms timer expired.\n");
		}
		if(__interval_1us_flag)
		{
			__interval_1us_flag = 0;
			++interval_1us_count;
			capture_1us = systick_count;
			printf("Status: 1us timer expired.\n");
		}
		if(__systick_flag)
		{
			__systick_flag = 0;
			++systick_count;
		}
	}

	printf("Timer test complete.\n");
	printf("- Measured %08X system ticks.\n", systick_count);
	printf("- Measured %08X millisecond ticks.\n", interval_1ms_count);
	printf("- Measured %08X microsecond ticks.\n", interval_1us_count);
	printf("- Captured systick by 1ms timer = %08X\n", capture_1ms);
	printf("- Captured systick by 1us timer = %08X\n", capture_1us);
	return 0;
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
