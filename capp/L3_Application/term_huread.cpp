


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
#include <string>
#include <vector>
using namespace std;

static void dump_memory(uint32_t address, uint32_t size);
extern Target_HuRead hureader;

/*----------------------------------------------------------------------------*/

enum terminal_colors : uint8_t
{
    C_RESET     = 0,
    C_RED       = 1,
    C_GREEN     = 2,
    C_YELLOW    = 3,
    C_BLUE      = 4,
    C_MAGENTA   = 5,
    C_CYAN      = 6,
    C_WHITE     = 7,
    C_GRAY      = 8,
    C_RED2      = 9,
    C_GREEN2    = 10,
    C_YELLOW2   = 11,
    C_BLUE2     = 12,
    C_MAGENTA2  = 13,
    C_CYAN2     = 14,
    C_WHITE2    = 15,
};

void text_color(uint8_t color)
{
    static uint8_t last_color = 0;

    if(color != last_color)
    {
        last_color = color;

        if(color == 0)
        {
            printf("\x1B[%dm", 0);
        }
        else
        {
            if(color & 8)
            {
                printf("\x1B[%dm", 90 + (color & 7));
            }
            else
            {
                printf("\x1B[%dm", 30 + (color & 7));
            }                
        }
    }
}


static int cmd_map_detect(int argc, char *argv[]);
static int cmd_map_drive(int argc, char *argv[]);
static int cmd_map_write(int argc, char *argv[]);
static int cmd_diagnostics(int argc, char *argv[]);
static int cmd_dump_space(int argc, char *argv[]);
static int cmd_dump_ten(int argc, char *argv[]);

static int cmd_list(int argc, char *argv[]);
static int cmd_exit(int argc, char *argv[]);
static int cmd_map(int argc, char *argv[]);
static int cmd_pop(int argc, char *argv[]);
static int cmd_poprom(int argc, char *argv[]);

static cli_cmd_t terminal_cmds[] = 
{
	{"diagnostics", cmd_diagnostics},
 	{"map_detect",  cmd_map_detect},
    {"map_drive",   cmd_map_drive},
    {"map_write",   cmd_map_write},
    {"dump_space",  cmd_dump_space},
    {"dump_ten",    cmd_dump_ten},

    {"map",       cmd_map},
    {"pop",       cmd_pop},
    {"rom",       cmd_poprom},
	{"?",         cmd_list},
	{"help",      cmd_list},
    {"exit",      cmd_exit},
 	{NULL,        NULL},
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
            hureader.set_data_pullups(pass ? 0xFF : 0x00);

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

    hureader.assert_reset(true);

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
            hureader.set_data_pullups(pass ? 0xFF : 0x00);

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




static int cmd_diagnostics(int argc, char *argv[])
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
        hureader.set_data_pullups(expected);
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
        hureader.set_data_pullups(expected);
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
        hureader.set_data_pullups(expected);
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


static int cmd_map_write(int argc, char *argv[])
{

    uint8_t temp;
    hureader.initialize();
    
    hureader.assert_hsm(false);
    hureader.assert_reset(false);
    hureader.set_card_detect_rx(true);
    hureader.set_irq2_rx(true);
    
    printf("Running read/write test by bank:\n");
    for(int bank = 0; bank < 0x100; bank++)
    {
        uint8_t state[2];
        uint8_t result;

        /* Set bank number to sample card detect pin at */
        hureader.set_address(bank << 13);


        hureader.set_data_pullups(0xFF);
        hureader.strobe_write(0xAA);
        state[0] = hureader.strobe_read();

        hureader.set_data_pullups(0x00);
        hureader.strobe_write(0x55);
        state[1] = hureader.strobe_read();

        if((bank & 0x0F) == 0x00)
        {
            printf("%02X: ", bank);
        }

        constexpr uint8_t kRoColor = C_BLUE2;
        constexpr uint8_t kRwColor = C_GREEN2;
        constexpr uint8_t kPartialColor = C_RED2;
        constexpr uint8_t kHizColor = C_YELLOW;

        /* Print result */
        if(state[0] == 0xAA && state[1] == 0x55)
        {
            text_color(kRwColor);
            printf("R/W ");    
            text_color(0);
        }
        else
        if(state[0] == 0xFF && state[1] == 0x00)
        {
                /* Data was not retained */
                text_color(kHizColor);
                printf("HIZ ");
                text_color(0);            
        }
        else
        {
            if(state[0] == state[1])
            {
                /* Data was not retained */
                text_color(kRoColor);
                printf("R/O ");
                text_color(0);
            }
            else
            {
                /* Data was partially retained */
                text_color(kPartialColor);
                printf("??? ");
                text_color(0);
            }
        }


        if((bank & 0x0F) == 0x0F)
        {
            printf("\n");
        }
    }
    printf("Status: Test complete.\n");
    return 1;
}








/*----------------------------------------------------------------------------*/
/* Sample CARD_DETECT# pin at each bank and report the state.
/*----------------------------------------------------------------------------*/

static int cmd_map_drive(int argc, char *argv[])
{

    uint8_t temp;
    hureader.initialize();
    
    hureader.assert_hsm(false);
    hureader.assert_reset(false);
    hureader.set_card_detect_rx(true);
    hureader.set_irq2_rx(true);



    printf("Running data bus drive test by bank:\n");
    for(int bank = 0; bank < 0x100; bank++)
    {
        uint8_t state[2];
        uint8_t result;

        /* Set bank number to sample card detect pin at */
        hureader.set_address(bank << 13);

        /* Sample data bus with pull-downs enabled */    
        hureader.set_data_pullups(0x00);
        state[0] = hureader.strobe_read();

        /* Sample data bus with pull-ups enabled */
        hureader.set_data_pullups(0xFF);
        state[1] = hureader.strobe_read();

        /* Compute delta */
        result = state[0] ^ state[1];

        if((bank & 0x0F) == 0x00)
        {
            printf("%02X: ", bank);
        }

        constexpr uint8_t kDrvColor = C_BLUE2;
        constexpr uint8_t kPartialColor = C_RED2;
        constexpr uint8_t kHizColor = C_YELLOW;

        /* Print result */
        if(state[0] == 0x00 && state[1] == 0xFF)
        {
            /* Data followed pull-ups exactly */
            text_color(kHizColor);
            printf("HIZ ");    
            text_color(0);
        }
        else
        {
            if(result)
            {
                /* Some bits followed pull-ups, others didn't */
                text_color(kPartialColor);
                printf("??? ");
                text_color(0);
            }
            else
            {
                /* No data followed the pull-ups */
                text_color(kDrvColor);
                printf("DRV ");
                text_color(0);
            }
        }


        if((bank & 0x0F) == 0x0F)
        {
            printf("\n");
        }
    }
    printf("Status: Test complete.\n");
    return 1;
}









/*----------------------------------------------------------------------------*/
/* Sample CARD_DETECT# pin at each bank and report the state.
/*----------------------------------------------------------------------------*/

static int cmd_map_detect(int argc, char *argv[])
{
    uint8_t temp;
    hureader.initialize();
    
    hureader.assert_hsm(false);
    hureader.assert_reset(false);
    hureader.set_irq2_rx(true);
    hureader.set_data_pullups(0xFF);

    printf("Card detect pin state by bank:\n");
    printf("Legend:\n");
    printf("* LOW : Card detect sense pin is driven low.\n");
    printf("* HI  : Card detect sense pin is driven high.\n");
    printf("* HIZ : Card detect sense pin is floating.\n");
    printf("* INV : Invalid state detected.\n");
    printf("* -- : .\n");
    for(int bank = 0; bank < 0x100; bank++)
    {
        bool result;

        /* Set bank number to sample card detect pin at */
        hureader.set_address(bank << 13);

        /* Record card detect state at this bank */
        hureader.set_card_detect_rx(false);
        result = hureader.get_card_detect();
        hureader.set_card_detect_rx(true);        
        result = (result << 1)  | (hureader.get_card_detect() & 0x01);

        if((bank & 0x0F) == 0x00)
        {
            printf("%02X: ", bank);
        }

        constexpr uint8_t kHizColor = C_YELLOW;
        constexpr uint8_t kLoColor = C_BLUE2;
        constexpr uint8_t kHiColor = C_GREEN2;
        constexpr uint8_t kInvColor = C_RED2;


        switch(result & 3)
        {
            case 0:
                text_color(kLoColor);
                printf("LOW ");
                text_color(0);
                break;
            case 1:
                text_color(kHizColor);
                printf("HIZ ");
                text_color(0);
                break;
            case 2:
                text_color(kInvColor);
                printf("INV ");
                text_color(0);
                break;
            case 3:
                text_color(kHiColor);
                printf("HI  ");
                text_color(0);
                break;
        }

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




#if 0

$2710 = last value for tma $10

// unlock backup memory
00000668: 43 10              tma  #$10
0000066A: 8D 10 27           sta  $2710
0000066D: A9 F7              lda  #$F7
0000066F: 53 10              tam  #$10
00000671: D3 79 E6 07 18 03  tin  $E679 $1807 $0003
00000678: 60                 rts

// unlock using 8000-9fff
00000686: 78                 sei
00000687: 43 10              tma  #$10      # save mpr 4
00000689: 8D 10 27           sta  $2710
0000068C: A9 68              lda  #$68
0000068E: 53 10              tam  #$10
00000690: 8D 00 80           sta  $8000     write 0x68 to 0x68:0000
00000693: A9 78              lda  #$78
00000695: 53 10              tam  #$10
00000697: A9 73              lda  #$73
00000699: 9C 00 80           stz  $8000     write 0x00 to 0x78:0000
0000069C: 8D 00 80           sta  $8000     write 0x73 to 0x78:0000
0000069F: 8D 00 80           sta  $8000     write 0x73 to 0x78:0000
000006A2: 8D 00 80           sta  $8000     write 0x73 to 0x78:0000
000006A5: A9 40              lda  #$40      # restore mpr 4
000006A7: 53 10              tam  #$10
000006A9: 60                 rts

// unlock using a000-bfff
000006AA: 78                 sei
000006AB: 43 20              tma  #$20
000006AD: 8D 11 27           sta  $2711
000006B0: A9 68              lda  #$68
000006B2: 53 20              tam  #$20
000006B4: 8D 00 A0           sta  $A000     write 0x68 to 0x68:0000
000006B7: A9 78              lda  #$78
000006B9: 53 20              tam  #$20
000006BB: A9 73              lda  #$73
000006BD: 9C 00 A0           stz  $A000     write 0x00 to 0x78:0000
000006C0: 8D 00 A0           sta  $A000     write 0x00 to 0x78:0000
000006C3: 8D 00 A0           sta  $A000     write 0x00 to 0x78:0000
000006C6: 8D 00 A0           sta  $A000     write 0x00 to 0x78:0000
000006C9: A9 40              lda  #$40      
000006CB: 53 20              tam  #$20
000006CD: 60                 rts


000006CE: A9 68              lda  #$68
000006D0: 53 10              tam  #$10
000006D2: 8D 00 80           sta  $8000     write 0x68 to 0x68:0000
000006D5: A9 78              lda  #$78
000006D7: 53 10              tam  #$10
000006D9: 9C 00 80           stz  $8000     write 0x00 to 0x78:0000
000006DC: A9 60              lda  #$60
000006DE: 53 10              tam  #$10
000006E0: 8D 00 80           sta  $8000     write 0x60 to 0x60:0000
000006E3: AD 10 27           lda  $2710
000006E6: 53 10              tam  #$10
000006E8: 58                 cli
000006E9: 60                 rts

000006EA: A9 68              lda  #$68
000006EC: 53 20              tam  #$20
000006EE: 8D 00 A0           sta  $A000     write 0x68 to 0x68:0000
000006F1: A9 78              lda  #$78
000006F3: 53 20              tam  #$20
000006F5: 9C 00 A0           stz  $A000     write 0x00 to 0x78:0000
000006F8: A9 60              lda  #$60
000006FA: 53 20              tam  #$20
000006FC: 8D 00 A0           sta  $A000     write 0x60 to 0x60:0000
000006FF: AD 11 27           lda  $2711
00000702: 53 20              tam  #$20
00000704: 58                 cli
00000705: 60                 rts


0000106F: 43 04              tma  #$04      # get mpr 2
00001071: 48                 pha            # push to stack
00001072: A9 04              lda  #$04      
00001074: 18                 clc
00001075: 65 03              adc  $03
00001077: 53 04              tam  #$04      # set mpr 2 to 0x04+0x03 = 0x07 in 4000-5FFF
00001079: 20 86 E6           jsr  $E686     # unlock
0000107C: 73 00 48 00 98 00  tii  $4800 $9800 $0800         8000-0fff
          08 
00001083: 20 CE E6           jsr  $E6CE     #lock
00001086: 68                 pla
00001087: 53 04              tam  #$04
00001089: 60                 rts

#endif

extern "C" void huread_page_asm(uint16_t *auto_port, uint8_t *buffer);
static int cmd_dump_space(int argc, char *argv[])
{
    uint8_t buffer[0x2000];

    hureader.initialize();
    hureader.assert_reset(true);
    hureader.assert_hsm(false);
    hureader.set_card_detect_rx(true);
    hureader.set_irq2_rx(true);
    hureader.set_data_pullups(0xFF);
    hureader.assert_reset(false);

    printf("Dumping HuCard memory space.\n");

    int fd = pc_fopen("hucard.bin", "wb");

    for(int bank = 0; bank < 0x100; bank++)
    {
        printf("Reading bank %02X ... ", bank);

        uint8_t *ptr = buffer;
        for(int page = 0; page < 0x20; page++)
        {
            hureader.set_address(bank * 0x2000 + page * 0x100 | 0xFF);
#if 1
            huread_page_asm(hureader.get_auto_port(), &buffer[page * 0x100]);
#else            
            for(int i = 0; i < 0x100; i++)
            {
                *ptr++ = hureader.strobe_read_auto();
            }
#endif            
        }
        printf("Transferring bank to host ... ");
        pc_fwrite(fd, buffer, 0x2000);
        printf(" complete.\n");
    }

    pc_fclose(fd);
    pc_exit();

    printf("Status: Operation complete.\n");
    return 1;
}



#if 0

// unlock using 8000-9fff
00000686: 78                 sei
00000687: 43 10              tma  #$10      # save mpr 4
00000689: 8D 10 27           sta  $2710
0000068C: A9 68              lda  #$68
0000068E: 53 10              tam  #$10
00000690: 8D 00 80           sta  $8000     write 0x68 to 0x68:0000
00000693: A9 78              lda  #$78
00000695: 53 10              tam  #$10
00000697: A9 73              lda  #$73
00000699: 9C 00 80           stz  $8000     write 0x00 to 0x78:0000
0000069C: 8D 00 80           sta  $8000     write 0x73 to 0x78:0000
0000069F: 8D 00 80           sta  $8000     write 0x73 to 0x78:0000
000006A2: 8D 00 80           sta  $8000     write 0x73 to 0x78:0000
000006A5: A9 40              lda  #$40      # restore mpr 4
000006A7: 53 10              tam  #$10
000006A9: 60                 rts
#endif

static int cmd_dump_ten(int argc, char *argv[])
{
    uint8_t buffer[0x2000];

    hureader.initialize();
    hureader.assert_reset(true);
    hureader.assert_hsm(false);
    hureader.set_card_detect_rx(true);
    hureader.set_irq2_rx(true);
    hureader.set_data_pullups(0xFF);
    hureader.assert_reset(false);


// unlocked ram has e0000-effff set to 0xfe - bad data?

    printf("Dumping HuCard memory space.\n");

    int fd = pc_fopen("hucard.bin", "wb");


    hureader.card_write_logical(0x68, 0x0000, 0x68);
    hureader.card_write_logical(0x78, 0x0000, 0x00);
    hureader.card_write_logical(0x78, 0x0000, 0x73);
    hureader.card_write_logical(0x78, 0x0000, 0x73);
    hureader.card_write_logical(0x78, 0x0000, 0x73);

    // 70-77 is weird

    for(int bank = 0x70; bank < 0x78; bank++)
    {
        printf("Reading bank %02X ... ", bank);

        uint8_t *ptr = buffer;
        for(int page = 0; page < 0x20; page++)
        {
#if 1

            hureader.set_address(bank * 0x2000 + page * 0x100 | 0xFF);
            huread_page_asm(hureader.get_auto_port(), &buffer[page * 0x100]);


#else            
            for(int i = 0; i < 0x100; i++)
            {
                *ptr++ = hureader.strobe_read_auto();
            }
#endif            
        }
        printf("Transferring bank to host ... ");
        pc_fwrite(fd, buffer, 0x2000);
        printf(" complete.\n");
    }


    hureader.card_write_logical(0x68, 0x0000, 0x68);
    hureader.card_write_logical(0x78, 0x0000, 0x00);
    hureader.card_write_logical(0x60, 0x0000, 0x60);

    pc_fclose(fd);
    pc_exit();

    printf("Status: Operation complete.\n");
    return 1;
}