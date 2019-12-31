


#include <stdio.h> // sprintf
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "../sbc.hpp"
#include "../L1_Peripheral/uart.hpp"
#include "../L3_Application/cli.hpp"
#include "../L3_Application/app.hpp"
#include "../L3_Application/term_diag.hpp"
#include "../newlib/mem_heap.hpp"
#include "term_main.hpp"
#include "term_flash.hpp"
#include "../L1_Peripheral/timer.hpp"
#include "../debug.h"
#include "../L1_Peripheral/parallel_memory.hpp"

extern void dump_memory(uint32_t address, uint32_t size);


static int cmd_info(int argc, char *argv[]);
static int cmd_list(int argc, char *argv[]);
static int cmd_exit(int argc, char *argv[]);

static cli_cmd_t terminal_cmds[] = 
{
	{"?",       cmd_list},
	{"help",    cmd_list},
	{"info",    cmd_info},
    {"exit",    cmd_exit},
	{NULL,      NULL},
};

/*----------------------------------------------------------------------------*/

int cmd_info(int argc, char *argv[])
{
    ParallelEEPROM dev;

    dev.set_base_address(0x200001);
    dev.get_information();

    printf("Device information:\n");
    printf("- Base address:    %08X\n", dev.base_address);
    printf("- Byte lane:       %s\n", dev.byte_lane ? "D[15:8]" : "D[7:0]");
    printf("- Manufacturer ID: %02X\n", dev.manufacturer_code);
    printf("- Device ID:       %02X\n", dev.device_code);
    printf("- Device size:     %08X (%dk)\n", dev.size, dev.size >> 10);

    return 1;
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

int cmd_flash(int argc, char *argv[])
{
    bool running = true;
    while(running)
    {
        int status = process_command_prompt(terminal_cmds, "flash:\\>");
        if(status == -1)
        {
            running = false;
            break;
        }
        WAIT_IRQ_ANY();
    }
	return 0;
}









volatile uint16_t *flash = (volatile uint16_t *)0x200000;

void flash_write_page(int channel, int offset, char *page)
{
	int shift = channel ? 8 : 0;

	/* SDP & Page-Write */
	flash[0x5555] = 0xAA << shift;
	flash[0x2AAA] = 0x55 << shift;
	flash[0x5555] = 0xA0 << shift;

	// delay

	for(int i = 0; i < 128; i++)
	{
		flash[offset] = page[0] << shift;	
	}

	// delay


}



void flash_sw_id_entry(int channel)
{
	int shift = channel ? 8 : 0;

	/* Software ID entry */
	flash[0x5555] = 0xAA << shift;
	flash[0x2AAA] = 0x55 << shift;
	flash[0x5555] = 0x90 << shift;
}



void flash_sw_id_exit(int channel)
{
	int shift = channel ? 8 : 0;

	/* Software ID entry */
	flash[0x5555] = 0xAA << shift;
	flash[0x2AAA] = 0x55 << shift;
	flash[0x5555] = 0xF0 << shift;
}


void flash_write(uint16_t value)
{
	const int page_size = 0x80;

	/* Page program */
	flash[0x5555] = 0xAAAA;	
	flash[0x2AAA] = 0x5555;	
	flash[0x5555] = 0xA0A0;	

	/* Load page buffer */
	for(int i = 0; i < page_size; i++)
	{
		flash[i] = value;
	}

	// FIXME: polling
	delay_ms(100);

	/* Reset */
	flash[0x5555] = 0xAAAA;	
	flash[0x2AAA] = 0x5555;	
	flash[0x5555] = 0xF0F0;	
}


void flash_erase(void)
{
	printf("Sending erase command...\n");

	flash[0x5555] = 0xAAAA;	
	flash[0x2AAA] = 0x5555;	
	flash[0x5555] = 0x8080;	

	flash[0x5555] = 0xAAAA;	
	flash[0x2AAA] = 0x5555;	
	flash[0x5555] = 0x1010;	

	printf("Waiting for erase to complete...\n");
	delay_ms(100);

	printf("Erase finished.\n");
}

void flash_info(void)
{
	printf("Flash information\n");

	for(int channel = 0; channel < 2; channel++)
	{
		uint16_t id[2];		
		int shift = channel ? 8 : 0;

		flash_sw_id_entry(channel);

		/* Read ID */
		id[0] = (flash[0] >> shift) & 0xFF;
		id[1] = (flash[1] >> shift) & 0xFF;

		flash_sw_id_exit(channel);

		const char *msg[] =
		{
			"Even, Upper, LSB",
			"Odd,  Lower, MSB"
		};

		printf("Device #%d (%s): ", channel, msg[channel]);

		if((id[0] & id[1]) == 0xFF)
		{
			printf("Device not present or not responding.\n");
		}
		else
		{
			printf("ID1:%02X", id[0]);
			switch(id[0])
			{
				case 0xBF:
					printf(" (%s)", "SST/Greenliant");
					break;
					
				default:
					break;
			}
			printf(" ID2:%02X", id[1]);
			printf("\n");
		}
	}
}



static int cmd_flash2(int argc, char *argv[])
{
	if(argc == 1)
	{
		printf("usage: flash <command>\n");
		printf("Available commands:\n");
		printf("- info  : Show device information.\n");
		printf("- erase : Erase device.\n");
		printf("- write : Write pattern to device.\n");
		printf("- read  : Read device.\n");
		return 0;
	}
	
	if(argc >= 2)
	{
		if(strcmp(argv[1], "info") == 0)
		{
			//
			flash_info();
		}
		else
		if(strcmp(argv[1], "erase") == 0)
		{
			//
			flash_erase();
		}
		else
		if(strcmp(argv[1], "write") == 0)
		{
			uint16_t value = 0xAA55;
			if(argc == 3)
			{
				value = strtol(argv[2], NULL, 16);
				printf("Set flash write pattern to %04X.\n", value);
			}
			flash_write(value);
		}
		else
		if(strcmp(argv[1], "read") == 0)
		{
			dump_memory(0x200000, 0x100);
		}
		else
		{
			printf("Unrecognized command `%s'.\n", argv[1]);
		}
	}

	return 0;
}




void dump_memory(uint32_t address, uint32_t size)
{
	int width = 0x10;
	char buf[80];
	char fragment[10];
	uint8_t *ptr = (uint8_t *)address;

	/* Print hex rows */	
	for(int row = 0; row < width; row++)
	{
		sprintf(buf, "%08X: ", (uint32_t)ptr);
		char *out = &buf[10];

		/* Write hex data */
		for(int col = 0; col < width; col++)
		{
			char ch = ptr[col];
			*out++ = "0123456789ABCDEF"[(ch >> 4) & 0x0F];
			*out++ = "0123456789ABCDEF"[(ch >> 0) & 0x0F];
			if(col & 1)
				*out++ = ' ';
		}

		/* Draw separator */
		*out++ = '|';
		*out++ = ' ';

		/* Write printable ASCII characters */
		for(int col = 0; col < width; col++)
		{
			uint8_t ch = ptr[col];
			if(isprint(ch) && ((ch & 0x80) == 0x00))
				*out++ = ch;
			else
				*out++ = '.';
		}

		/* Terminate line and advance pointer */
		*out++ = 0;
		ptr += width;

		/* Print complete line to terminal */
		printf("%s\n", buf);
	}	
}



