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
#include "L3_Application/cli.hpp"
#include "L3_Application/mutex.hpp"
#include "cli_cmd.hpp"
#include "newlib/printf.hpp"
#include "newlib/mem_heap.hpp"
#include "debug.h"

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

typedef void (*ctor_func_ptr)(void);

extern "C" void startup(void)
{
	/* Run all default constructors */
	extern uint32_t __CTOR_LIST__;
	uint32_t *ctor_list = (uint32_t *)&__CTOR_LIST__;

	for(uint32_t i = ctor_list[0]; i > 0; i--)
	{
		((ctor_func_ptr)ctor_list[i])();
	}
}

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

extern cli_cmd_t terminal_cmds[];

int main(void)
{
	/* Initialize board peripherals */
	sbc_initialize();

	/* Enable interrupts */
	EXIT_CRITICAL_SECTION();

	/* Display banner */
	printf("\n\nRAM program\nBuild time: %s\nBuild date: %s\n\n", __TIME__, __DATE__);

	/* Set POST code */
	system_controller.set_post(0xA5);

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
