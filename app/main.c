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
#include "sbc.h"
#include "timer.h"
#include "ringbuf.h"
#include "cli.h"
#include "cli_cmd.h"

#define RESET()						__asm__ __volatile__("reset\n")
#define ENTER_CRITICAL_SECTION()	__asm__ __volatile__("move.w\t#0x2700, %sr\n")
#define EXIT_CRITICAL_SECTION()		__asm__ __volatile__("move.w\t#0x2000, %sr\n")
#define WAIT_IRQ_ANY()				__asm__ __volatile__("stop\t#0x2000\n")


/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

extern cli_cmd_t terminal_cmds[];

int main(void)
{
	/* Display banner */
	printf("\n\nRAM program\nBuild time: %s\nBuild date: %s\n\n", __TIME__, __DATE__);

	/* Set POST state */
	set_post(0xA5);

	/* Set up PIT 82C54 */
	timer_init();

	/* Clear pending interrupts */
	REG_IPEND_CLR[0] = 0xFF;

	/* Enable interrupt levels 7, 6, 3 */
	REG_IENABLE[0] = 0xC8;

	/* Initialize TX/RX ring buffers */
	ringbuf_init(&uart_rx_ringbuf, RINGBUF_MAX_SIZE);
	ringbuf_init(&uart_tx_ringbuf, RINGBUF_MAX_SIZE);

	/* Enable interrupts */
	EXIT_CRITICAL_SECTION();

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
