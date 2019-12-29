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

enum ApplicationSection : uint8_t
{
	kTEXT,
	kDATA,
	kBSS,
	kKERNEL,
	kSTACK,
	kHEAP
};

struct section_info_tag
{
	uint32_t start;
	uint32_t end;
	uint32_t size;
	char name[32];
};

typedef section_info_tag section_info_t;

class Application
{
public:

	bool get_section_info(ApplicationSection which, section_info_t &record)
	{
		extern char _text_start;
		extern char _etext;
		extern uint32_t _stext;
		extern char _data_start;
		extern char _edata;
		extern uint32_t _sdata;
		extern char _bss_start;
		extern char _bend;
		extern uint32_t _sbss;
		extern char _kernel_start;
		extern char _kernel_end;
		extern uint32_t _skernel;

		constexpr uint32_t kWorkRAMEnd = 0x200000; /* Get from SBC */
		constexpr uint32_t kStackTop   = 0x110000;

		switch(which)
		{
			case kTEXT:
				record.start = (uint32_t)&_text_start;
				record.end = (uint32_t)&_etext;
				record.size = (uint32_t)&_stext;
				strcpy(record.name, "TEXT");
				return true;

			case kDATA:
				record.start = (uint32_t)&_data_start;
				record.end = (uint32_t)&_edata;
				record.size = (uint32_t)&_sdata;
				strcpy(record.name, "DATA");
				return true;

			case kBSS:
				record.start = (uint32_t)&_bss_start;
				record.end = (uint32_t)&_bend;
				record.size = (uint32_t)&_sbss;
				strcpy(record.name, "BSS");
				return true;

			case kKERNEL:
				record.start = (uint32_t)&_kernel_start;
				record.end = (uint32_t)&_kernel_end;
				record.size = (uint32_t)&_skernel;
				strcpy(record.name, "KERNEL");
				return true;

			case kHEAP:
				record.start = (uint32_t )sbrk(0);
				record.end = kWorkRAMEnd;
				record.size = record.end - record.start;
				strcpy(record.name, "HEAP");
				return true;

			case kSTACK:
				record.start = get_sp();
				record.end = kStackTop;
				record.size = record.end - record.start;
				strcpy(record.name, "STACK");
				return true;

			default:
				record.start = 0;
				record.end = 0;
				record.size = 0;
				strcpy(record.name, "<INVALID>");
				return false;
		}
	}
};

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

	Application app;
	section_info_t sec;

	for(int i = 0; ; i++)
	{
		if(!app.get_section_info((ApplicationSection)i, sec))
			break;
		printf("[%s]\n", sec.name);
		printf("sec start = %08X\n", sec.start);
		printf("sec end   = %08X\n", sec.end);
		printf("sec size  = %08X (%dk)\n", sec.size, sec.size >> 10);
	}

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
