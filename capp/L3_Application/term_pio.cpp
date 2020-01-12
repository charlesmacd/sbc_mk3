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
#include "../L3_Application/term_pio.hpp"
#include "../L1_Peripheral/sbc_pio.hpp"
#include "../newlib/mem_heap.hpp"
#include "term_main.hpp"
#include "../debug.h"


static int cmd_list(int argc, char *argv[]);
static int cmd_exit(int argc, char *argv[]);
static int cmd_pioram(int argc, char *argv[]);
static int cmd_power(int argc, char *argv[]);;
static int cmd_spi(int argc, char *argv[]);
static int cmd_test(int argc, char *argv[]);

static cli_cmd_t terminal_cmds[] = 
{
	{"?",       cmd_list},
	{"help",    cmd_list},
    {"exit",    cmd_exit},
	{"pioram", 	cmd_pioram},
	{"power", 	cmd_power},
	{"spi", 	cmd_spi},
	{"test", 	cmd_test},
	{NULL,      NULL},
};


/*----------------------------------------------------------------------------*/

int cmd_power(int argc, char *argv[])
{
	
	return 1;
}

/*----------------------------------------------------------------------------*/

class soft_spi_interface
{
private:
	static constexpr uint8_t kSCLK = 0x00;
	static constexpr uint8_t kMOSI = 0x01;
	static constexpr uint8_t kSSEL = 0x02;
	static constexpr uint8_t kRST  = 0x03;
	static constexpr uint8_t kMISO = 0x07;

	uint8_t clock_polarity;
	uint8_t clock_phase;
public:
	SBCPIO_EXT &intf;

	soft_spi_interface(SBCPIO_EXT &intf_) : intf(intf_)
	{
	
	}

	void configure(uint8_t mode)
	{
		clock_polarity = (mode >> 0) & 1;
		clock_phase = (mode >> 1) & 1;
		intf.write(kSCLK, clock_polarity);
		intf.write(kMOSI, 0);
		intf.write(kSSEL, 1);
	}


	void clock(void)
	{
		intf.write(kSCLK, clock_polarity ^ 1);
		intf.write(kSCLK, clock_polarity ^ 0);
	}

	uint8_t exchange(uint8_t value)
	{
		uint8_t temp = 0;
		for(int i = 0; i < 8; i++)
		{
			intf.write(kMOSI, value >> (i ^ 7));
			intf.write(kSCLK, clock_polarity ^ 1);
			temp |= intf.read_bit(kMISO, 0) << (i);
			intf.write(kSCLK, clock_polarity ^ 0);
		}		
		return temp;		
	}

	uint8_t read(void)
	{
		uint8_t temp = 0;
		
		intf.write(kMOSI, 1);

		for(int i = 0; i < 8; i++)
		{
			intf.write(kSCLK, clock_polarity ^ 1);
			temp |= intf.read_bit(kMISO, 0) << (i);
			intf.write(kSCLK, clock_polarity ^ 0);
		}		
		return temp;		
	}

	void write(uint8_t value)
	{
		for(int i = 0; i < 8; i++)
		{
			intf.write(kMOSI, (value >> (i ^ 7)) ^ 1);
			clock();
		}		
	}

	void select(bool is_asserted)
	{
		intf.write(kSSEL, is_asserted ? 0 : 1);
	}

	void reset(bool is_asserted)
	{
		intf.write(kRST, is_asserted ? 0 : 1);
	}
};

int cmd_spi(int argc, char *argv[])
{
	int v;
	int k;

	// hack of sorts 
	SBCPIO_EXT &diag = sbc_pio.ext;
	diag.initialize();

	soft_spi_interface spi(sbc_pio.ext);

	// set mode and state
	spi.configure(0);

	// pulse reset
	spi.reset(true);
	spi.reset(false);

	// transfer
	spi.select(true);
	spi.write(0xAA);
	v = spi.read();
	k = spi.exchange(0xdb);
	spi.select(false);

	printf("v=%08X\n", v);
	printf("v=%08X\n", k);
	
	return 1;
}

int cmd_test(int argc, char *argv[])
{
	static constexpr uint8_t kDataBit = 0;

	// hack of sorts 
	SBCPIO_EXT &diag = sbc_pio.ext;

	diag.initialize();

	/* I/O readback test */
	for(int i = 0; i < 8; i++)
	{
		uint8_t temp = 0;
		
		/* Drive latch bit low */
		diag.write(i, 0x00);
		temp = (temp << 1) | diag.read_bit(i, kDataBit);

		/* Drive latch bit high */		
		diag.write(i, 0x01);
		temp = (temp << 1) | diag.read_bit(i, kDataBit);

		/* Reset latch bit */
		diag.write(i, kDataBit);

		/* Report bit state */
		printf("- Latch bit %d is ", i);
		switch(temp & 3)
		{
			case 0: /* L -> L */
				printf("fixed low. (pull down?)");
				break;
			case 1: /* L -> H */
				printf("modifiable.");
				break;
			case 2: /* H -> L */
				printf("modifiable, inverted.");
				break;
			case 3: /* H -> H */
				printf("fixed high. (pull up?)");
				break;
		}
		printf("\n");
	}
	printf("Status: Test complete.\n");
	return 1;
}


/*----------------------------------------------------------------------------*/

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
//#endif

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

int cmd_pio(int argc, char *argv[])
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
