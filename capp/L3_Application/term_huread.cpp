


#include <stdio.h> // sprintf
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "../sbc.hpp"
#include "../L1_Peripheral/uart.hpp"
#include "../L3_Application/cli.hpp"
#include "../L3_Application/app.hpp"
#include "../newlib/mem_heap.hpp"
#include "term_main.hpp"
#include "term_huread.hpp"
#include "../L1_Peripheral/timer.hpp"
#include "../L1_Peripheral/target_huread.hpp"
#include "../debug.h"
#include "../L1_Peripheral/parallel_memory.hpp"

static void dump_memory(uint32_t address, uint32_t size);
extern Target_HuRead hureader;

static int cmd_detect(int argc, char *argv[]);
static int cmd_list(int argc, char *argv[]);
static int cmd_exit(int argc, char *argv[]);
static int cmd_diag(int argc, char *argv[]);
static int cmd_map(int argc, char *argv[]);
static int cmd_pop(int argc, char *argv[]);
static int cmd_poprom(int argc, char *argv[]);

static cli_cmd_t terminal_cmds[] = 
{
	{"?",       cmd_list},
	{"diag",    cmd_diag},
	{"help",    cmd_list},
	{"detect",  cmd_detect},
    {"exit",    cmd_exit},
    {"map",     cmd_map},
    {"pop",     cmd_pop},
    {"rom",     cmd_poprom},
	{NULL,      NULL},
};

#include "crc32.c"

/*----------------------------------------------------------------------------*/

#pragma GCC push_options
#pragma GCC optimize ("O3")

static int cmd_map(int argc, char *argv[])
{
    uint8_t map[0x100];

    hureader.initialize();


    printf("Start of mapping test.\n");

    for(int bank = 0; bank < 0x100; bank++)
    {
        const uint32_t page_size = 0x10;
        uint8_t page[2][page_size];

        map[bank] = '?';

        printf("Checking bank %02X ...\n", bank);

        for(int pass = 0; pass < 2; pass++)
        {
            printf("- Pass %d of %d\n", 1+pass, 2);
            hureader.set_dbrx(pass ? 0xFF : 0x00);

            for(int i = 0; i < page_size; i++)
            {
                hureader.set_address(bank * 0x2000 + i);
                page[pass][i] = hureader.strobe_read();
            }        
        }

        //pop
        // 00-3f : rom
        // 40-5f : ram
        // 60-ff : not consistent

        // acdpro
        // 00-3F : rom
        // 40-43 : rom (counts as, actually data dreg mmio)
        // 44-67 : unmapped
        // 68-7F : ram
        // 80-FF : unmapped (* for io)

        // sf2
        // 00-3F : ROM
        // 50-FF : unmapped (hiz until mapped?)

        if(memcmp(&page[0][0], &page[1][0], page_size) == 0)
        {
            printf("* Bank %02X is actively driven.\n", bank);
            uint8_t result[2];            
            hureader.strobe_write(0x55);
            result[0] = hureader.strobe_read();
            hureader.strobe_write(0xAA);
            result[1] = hureader.strobe_read();
            if(result[0] == 0x55 && result[1] == 0xaa)
            {
                printf("- Bank %02X is RAM\n", bank);
                map[bank] ='W';
            }
            else
            {
             map[bank] = 'R';
            }

        }
        else
        {
            printf("* Bank %02X is not driven consistently.\n", bank);
        }
    }

    for(int bank = 0; bank < 0x100; bank++)
    {
        if((bank & 0x0F) == 0x00)
        {
            printf("%02X: ", bank);
        }

        printf("%c ", map[bank]);

        if((bank & 0x0F) == 0x0F)
        {
            printf("\n");
        }
    }

    printf("Mapping test complete.\n");
    return 1;
}


/*----------------------------------------------------------------------------*/


void write_bank(uint8_t bank, uint16_t offset, uint8_t data)
{
    hureader.set_address(bank * 0x2000 + offset);
    hureader.strobe_write(data);
}

uint8_t read_bank(uint8_t bank, uint16_t offset)
{
    hureader.set_address(bank * 0x2000 + offset);
    return hureader.strobe_read();
}

static int cmd_pop(int argc, char *argv[])
{
    hureader.initialize();

    for(int bank = 0x40; bank < 0x60; bank++)
    {
        printf("Clearing bank %02X ...\n", bank);
        for(int offset = 0; offset < 0x2000; offset++)
        {
            write_bank(bank, offset, 0xAA);
       }
    }

    printf("Writing test pattern ...\n");
    write_bank(0x40, 0x0000, 0xDE);
    write_bank(0x40, 0x0001, 0xAD);
    write_bank(0x40, 0x0002, 0xBE);
    write_bank(0x40, 0x0003, 0xEF);

    hureader.set_reset(true);

    printf("Writing test pattern ...\n");
    write_bank(0x40, 0x0000, 0x44);
    write_bank(0x40, 0x0001, 0x55);
    write_bank(0x40, 0x0002, 0x66);
    write_bank(0x40, 0x0003, 0x77);

    printf("Reading bank data ...\n");
    for(int bank = 0x40; bank < 0x60; bank++)
    {
        uint8_t result[4];
        result[0] = read_bank(bank, 0x0000);
        result[1] = read_bank(bank, 0x0001);
        result[2] = read_bank(bank, 0x0002);
        result[3] = read_bank(bank, 0x0003);

        printf("Bank=%02X | %02X %02X %02X %02X\n", bank, result[0], result[1], result[2], result[3]);
    }

    printf("End of pop test.\n");
    return 1;

    printf("Start of mapping test.\n");

    for(int bank = 0; bank < 0x100; bank++)
    {
        const uint32_t page_size = 0x10;
        uint8_t page[2][page_size];

        printf("Checking bank %02X ...\n", bank);

        for(int pass = 0; pass < 2; pass++)
        {
            printf("- Pass %d of %d\n", 1+pass, 2);
            hureader.set_dbrx(pass ? 0xFF : 0x00);

            for(int i = 0; i < page_size; i++)
            {
                hureader.set_address(bank * 0x2000 + i);
                page[pass][i] = hureader.strobe_read();
            }        
        }

        // 00-3f : rom
        // 40-5f : ram
        // 60-ff : not consistent

        // 40-43 is ram 32k
        // repeats through 5f


        if(memcmp(&page[0][0], &page[1][0], page_size) == 0)
        {
            printf("* Bank %02X is actively driven.\n", bank);
            uint8_t result[2];            
            hureader.strobe_write(0x55);
            result[0] = hureader.strobe_read();
            hureader.strobe_write(0xAA);
            result[1] = hureader.strobe_read();
            if(result[0] == 0x55 && result[1] == 0xaa)
            {
                printf("- Bank %02X is RAM\n", bank);
            }

        }
        else
        {
            printf("* Bank %02X is not driven consistently.\n", bank);
        }
    }
    printf("Mapping test complete.\n");
    return 1;
}

#pragma GCC pop_options



/*----------------------------------------------------------------------------*/


static int cmd_poprom(int argc, char *argv[])
{
    uint8_t buf[0x2000];
    hureader.initialize();
    uint32_t crc;
    

    printf("Reading bank data ...\n");
    for(int bank = 0x00; bank < 0x40; bank++)
    {
        printf("Reading bank %02X ... ", bank);

        for(int i = 0; i < 0x2000; i++)
        {
            buf[i] = read_bank(bank, i);
        }

        printf(" Calculating CRC ... ");
        crc = 0;
        crc = calculate_crc32c(crc, buf, 0x2000);

        printf("%08X\n", crc);
#if 0
    for(int i = 0; i < 0x10; i++)
    {
        printf("%02X ", buf[i]);
    }
    printf("]\n");
    #endif
#if 0
calculate_crc32c(uint32_t crc32c,
    const unsigned char *buffer,
    unsigned int length)
{
#endif

    }

    printf("End of pop test.\n");
    return 1;
};




static int cmd_diag(int argc, char *argv[])
{
    bool error;
    uint8_t temp;
    bool state[2];
    hureader.initialize();

    printf("\nStarting diagnostics.\n");

    /* Test CD# */

    printf("Testing CD# readback:\n");

    hureader.set_card_detect_rx(false);
    state[0] = hureader.get_card_detect();

    hureader.set_card_detect_rx(true);
    state[1] = hureader.get_card_detect();

    if(state[0] == false && state[1] == true)
    {
        printf("- CD# readback OK.\n");
    }
    else
    {
        printf("- CD# readback NG. (expected 0,1 got %d,%d)\n",
            state[0],
            state[1]);
        if(!state[0] && !state[1])
        {
            printf("- CD# is stuck low, is a HuCard inserted?\n");
        }
    }
    printf("\n");

    /* Test IRQ2# */

    printf("Testing IRQ2# readback:\n");

    hureader.set_irq2_rx(false);
    state[0] = hureader.get_irq2();

    hureader.set_irq2_rx(true);
    state[1] = hureader.get_irq2();

    if(state[0] == false && state[1] == true)
    {
        printf("- IRQ2# readback OK.\n");
    }
    else
    {
        printf("- IRQ2# readback NG. (expected 0,1 got %d,%d).\n",
            state[0],
            state[1]);
    }
    printf("\n");

    /* Test data bus */
    printf("Testing data bus:\n");

    printf("Test 1: Individual bits, positive:\n");
    error = false;
    for(int i = 0; i < 8 && !error; i++)
    {
        uint8_t expected = 1 << i;
        hureader.set_dbrx(expected);
        temp = hureader.read_dbus();
        if(temp != expected)
        {
            printf("- Data bus NG; Mismatch, expected %02X and read %02X.\n", expected, temp);
            error = true;
            break;
        }
    }
    if(!error)
    {
        printf("- Data bus OK.\n");
    }

    printf("Test 2: Individual bits, negative:\n");
    error = false;
    for(int i = 0; i < 8 && !error; i++)
    {
        uint8_t expected = ~(1 << i);
        hureader.set_dbrx(expected);
        temp = hureader.read_dbus();
        if(temp != expected)
        {
            printf("- Data bus NG; Mismatch, expected %02X and read %02X.\n", expected, temp);
            error = true;
            break;
        }
    }
    if(!error)
    {
        printf("- Data bus OK.\n");
    }

    printf("Test 3: All values:\n");
    error = false;
    for(int i = 0; i < 0x100 && !error; i++)
    {
        uint8_t expected = i;
        hureader.set_dbrx(expected);
        temp = hureader.read_dbus();
        if(temp != expected)
        {
            printf("- Data bus NG; Mismatch, expected %02X and read %02X.\n", expected, temp);
            error = true;
            break;
        }
    }
    if(!error)
    {
        printf("- Data bus OK.\n");
    }
    printf("\n");

    printf("End of diagnostics.\n");

    return 1;
}

/*----------------------------------------------------------------------------*/

static int cmd_detect(int argc, char *argv[])
{
    uint8_t temp;
    hureader.initialize();
    
    hureader.set_card_detect_rx(true);
    hureader.set_irq2_rx(true);
    hureader.set_dbrx(0xFF);

    printf("Card detect map by bank:\n");
    for(int bank = 0; bank < 0x100; bank++)
    {
        bool result;
        hureader.set_address(bank << 13);
        result = hureader.get_card_detect();

        if((bank & 0x0F) == 0x00)
        {
            printf("%02X: ", bank);
        }

        printf("%s ", result ? "--" : "CD");

        if((bank & 0x0F) == 0x0F)
        {
            printf("\n");
        }
    }

#if 0
   	hureader.set_card_detect_rx(false);
	printf("Drive CD# lo, readback: %d\n", hureader.get_card_detect());
	hureader.set_card_detect_rx(true);
	printf("Drive CD# hi, readback: %d\n", hureader.get_card_detect());

	hureader.set_irq2_rx(false);
	printf("Drive IRQ2# lo, readback: %d\n", hureader.get_irq2());
	hureader.set_irq2_rx(true);
	printf("Drive IRQ2# hi, readback: %d\n", hureader.get_irq2());

	temp = hureader.get_card_detect();
	printf("CD#   = %d\n", temp);
	temp = hureader.get_irq2();
	printf("IRQ2# = %d\n", temp);
#endif


    return 1;
}

/*----------------------------------------------------------------------------*/

static int cmd_exit(int argc, char *argv[])
{
    return -1;
}

/*----------------------------------------------------------------------------*/

static int cmd_list(int argc, char *argv[])
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

int cmd_huread(int argc, char *argv[])
{
    bool running = true;
    while(running)
    {
        int status = process_command_prompt(terminal_cmds, "huread:\\>");
        if(status == -1)
        {
            running = false;
            break;
        }
        WAIT_IRQ_ANY();
    }
	return 0;
}









static void dump_memory(uint32_t address, uint32_t size)
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



