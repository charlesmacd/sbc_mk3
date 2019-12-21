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
#include <assert.h>
#include "sbc.hpp"
#include "timer.hpp"
#include "L1_Peripheral/interrupt_controller.hpp"
#include "L1_Peripheral/post.hpp"
#include "L3_Application/cli.hpp"
#include "L3_Application/mutex.hpp"
//#include "L3_Application/ringbuf.h"
#include "L3_Application/ring_buf.hpp"
#include "cli_cmd.hpp"
#include "printf.hpp"
#include "debug.h"

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

extern cli_cmd_t terminal_cmds[];



//----------------------------------

extern "C" void __cxa_pure_virtual() { debug_putch('$'); while (1); }

//----------------------------------

RingBuffer q;

int main(void)
{
	sbc_initialize();

	/* Enable interrupts */
	EXIT_CRITICAL_SECTION();

	/* Display banner */
	printf("\n\nRAM program\nBuild time: %s\nBuild date: %s\n\n", __TIME__, __DATE__);
	puts("foobar");

	post.set(0xA5);

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
