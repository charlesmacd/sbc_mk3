/*
	File:
		main.c
	Author:
		Charles MacDonald
	Notes:
		None
*/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include "sbc.hpp"
#include "timer.hpp"
#include "peripheral/interrupt_controller.hpp"
#include "peripheral/post.hpp"

#include "utility/ringbuf.h"
#include "application/cli.hpp"
#include "cli_cmd.hpp"
#include "printf.hpp"

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

extern cli_cmd_t terminal_cmds[];


int main(void)
{
	sbc_initialize();

	/* Display banner */
	printf("\n\nRAM program\nBuild time: %s\nBuild date: %s\n\n", __TIME__, __DATE__);

	//post.set(0xce);


	/* Enable interrupts */
	EXIT_CRITICAL_SECTION();

	*((volatile uint8_t *)0xFFFF8001) = 0x5a;
	post.set(0xce);

	while(1)
	{
		/* Process UART commands */
		process_command_prompt(terminal_cmds);

		/* Idle until an event occurs */
		WAIT_IRQ_ANY();
	}
}


/*-------------------------------------------------------------------------------*/
/* End */
/*-------------------------------------------------------------------------------*/
