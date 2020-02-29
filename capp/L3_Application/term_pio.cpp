/*
	File: term_diag.cpp


	have genric func to get info about each pin for tester pov
	need generic func to control pin from base pio pov
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
#include "../L1_Peripheral/host_spi_interface.hpp"
#include "../L1_Peripheral/pioext_spi_interface.hpp"
#include "../L1_Peripheral/mcp23s17.hpp"
#include "../newlib/mem_heap.hpp"
#include "term_main.hpp"
#include "../debug.h"

#if 0
enum PIOSignalName : uint8_t
{
	kTD0, kTD1, kTD2, kTD3, kTD4, kTD5, kTD6, kTD7,
	kTD8, kTD9, kTD10, kTD11, kTD12, kTD13, kTD14, kTD15,
	kTA1, kTA2, kTA3, kTA4, kTA5, kTA6, kTA7,
	kTA8, kTA9, kTA10, kTA11, kTA12, kTA13, kTA14, kTA15,
	kTA16, kTA17, kTA18, kTA19, kTA20, kTA21, kTA22, kTA23,
	kTAS, kTRW, kTLDS, kTUDS,
	kTRESETI, kTHALTI,
	kTRESETO, kTHALTO,
	kTBR, kTBGACK, kTBG,
	kTFC0, kTFC1, kTFC2,
	kTIPL2, kTIPL1, kTIPL0,
	kTVPA, kTVMA, kTE, kTCLK,
	kTDTACK, kTBERR,
	kTPS_EN,
	kTCLKSEL0, kTCLKSEL1,
	kOUT3, kOUT6,

	kUM_TXE, kUM_RXF, kUM_PWE,
	kUSER_LED,
	kTPS_FLT,
	kFF_CKO,
	kWAITREQ
};
#endif




struct expander_pio_mapping_tag
{
	uint8_t address;
	MCP23S17_Port port;
	uint8_t bit;
	PIOSignalName signal; // use to look up durectuib
};

typedef expander_pio_mapping_tag expander_pio_mapping_t;


class PIOTester
{


public:
	
	mcp23s17 &expander;


	PIOTester(mcp23s17 &expander_) : expander(expander_) 
	{
		
	}

	static constexpr expander_pio_mapping_t expander_pio_mapping[] =
	{
		/* Device #0 */
		{0, PORTA, 0, kTD0},
		{0, PORTA, 1, kTD1},
		{0, PORTA, 2, kTD2},
		{0, PORTA, 3, kTD3},
		{0, PORTA, 4, kTD4},
		{0, PORTA, 5, kTD5},
		{0, PORTA, 6, kTD6},
		{0, PORTA, 7, kTD7},

		{0, PORTB, 0, kTD8},
		{0, PORTB, 1, kTD9},
		{0, PORTB, 2, kTD10},
		{0, PORTB, 3, kTD11},
		{0, PORTB, 4, kTD12},
		{0, PORTB, 5, kTD13},
		{0, PORTB, 6, kTD14},
		{0, PORTB, 7, kTD15},

		/* Device #1 */
		{0, PORTA, 0, kTCLK},
		{0, PORTA, 1, kTDTACK},
		{0, PORTA, 2, kTBERR},
		{0, PORTA, 3, kTVPA},
		{0, PORTA, 4, kTVMA},
		{0, PORTA, 5, kTE},
		{0, PORTA, 6, kTHALTI},		/* 1.GPBA6 */
		{0, PORTA, 7, kTRESETI},	/* 1.GPBA7 */
		{0, PORTA, 6, kTHALTO},		/* 1.GPBA6 */
		{0, PORTA, 7, kTRESETO},	/* 1.GPBA7 */

		{0, PORTB, 0, kTAS},
		{0, PORTB, 1, kTUDS},
		{0, PORTB, 2, kTLDS},
		{0, PORTB, 3, kTRW},
		{0, PORTB, 4, kTBG},
		{0, PORTB, 5, kTBGACK},
		{0, PORTB, 6, kTBR},
		/* 1.GPB7 unused */

		/* Device #2 */
		{2, PORTA, 0, kTA16},
		{2, PORTA, 1, kTA17},
		{2, PORTA, 2, kTA18},
		{2, PORTA, 3, kTA19},
		{2, PORTA, 4, kTA20},
		{2, PORTA, 5, kTA21},
		{2, PORTA, 6, kTA22},
		{2, PORTA, 7, kTA23},

		{2, PORTB, 0, kTIPL2},
		{2, PORTB, 1, kTIPL1},
		{2, PORTB, 2, kTIPL0},
		{2, PORTB, 3, kTFC2},
		{2, PORTB, 4, kTFC1},
		{2, PORTB, 5, kTFC0},
		/* GPB6 unused */
		/* GPB7 unused */

		/* Device #3 */
		/* GPA0 grounded (!) */
		{3, PORTA, 1, kTA1},
		{3, PORTA, 2, kTA2},
		{3, PORTA, 3, kTA3},
		{3, PORTA, 4, kTA4},
		{3, PORTA, 5, kTA5},
		{3, PORTA, 6, kTA6},
		{3, PORTA, 7, kTA7},

		{3, PORTB, 0, kTA8},
		{3, PORTB, 1, kTA9},
		{3, PORTB, 2, kTA10},
		{3, PORTB, 3, kTA11},
		{3, PORTB, 4, kTA12},
		{3, PORTB, 5, kTA13},
		{3, PORTB, 6, kTA14},
		{3, PORTB, 7, kTA15},
	};

	static constexpr PIOSignalName fieldData[] =  {
		kTD0,  kTD1,  kTD2,  kTD3,
		kTD4,  kTD5,  kTD6,  kTD7,
		kTD8,  kTD9,  kTD10, kTD11,
		kTD12, kTD13, kTD14, kTD15
	};


	static constexpr PIOSignalName fieldAddress[] =  {
		       kTA1,  kTA2,  kTA3,
		kTA4,  kTA5,  kTA6,  kTA7,
		kTA8,  kTA9,  kTA10, kTA11,
		kTA12, kTA13, kTA14, kTA15,
		kTA16, kTA17, kTA18, kTA19,
		kTA20, kTA21, kTA22, kTA23,
	};

	static constexpr PIOSignalName fieldFunctionCode[] = {
		kTFC0, kTFC1, kTFC2
	};

	static constexpr PIOSignalName fieldInterruptPriorityLevel[] = {
		kTIPL0, kTIPL1, kTIPL2
	};



	static constexpr int kNumExpanderPioMappingEntries = sizeof(expander_pio_mapping) / sizeof(expander_pio_mapping_t);
	uint8_t SignalToIndexMap[0x100];

	const expander_pio_mapping_t &getMapping(PIOSignalName id)
	{
		return expander_pio_mapping[ SignalToIndexMap[id] ];
	}
	uint8_t getPort(PIOSignalName id)
	{
		return getMapping(id).port;
	}
	uint8_t getBit(PIOSignalName id)
	{
		return getMapping(id).bit;
	}
	uint8_t getAddress(PIOSignalName id)
	{
		return getMapping(id).address;
	}

	void setPin(PIOSignalName id, bool is_high)
	{
//		expander_pio_mapping_t *mapping = &expander_pio_mapping[ SignalToIndexMap[id] ];

		const expander_pio_mapping_t &map = getMapping(id);

//		expander.write_reg(map.address, map.port ? 0x0D : 0x0C, 

	}

	void initialize(void)
	{
		/* Initialize signal to index map */
		for(int i = 0; i < kNumExpanderPioMappingEntries; i++)
		{
			SignalToIndexMap[ expander_pio_mapping[i].signal ] = i;
		}	
	}
};



static int cmd_list(int argc, char *argv[]);
static int cmd_exit(int argc, char *argv[]);
static int cmd_power(int argc, char *argv[]);
static int cmd_test_pio(int argc, char *argv[]);
static int cmd_test_pioram(int argc, char *argv[]);
static int cmd_test_interface(int argc, char *argv[]);
static int cmd_detect_expander(int argc, char *argv[]);

static cli_cmd_t terminal_cmds[] = 
{
	{"?",       		cmd_list},
	{"help",    		cmd_list},
    {"exit",    		cmd_exit},
	{"power", 			cmd_power},
	{"test_pio", 		cmd_test_pio},
	{"test_pioram", 	cmd_test_pioram},
	{"test_interface", 	cmd_test_interface},
	{"detect_expander", cmd_detect_expander},
	{NULL,      NULL},
};


/*----------------------------------------------------------------------------*/

int cmd_power(int argc, char *argv[])
{
	
	return 1;
}

/*============================================================================*/
/*============================================================================*/

/*============================================================================*/
/*============================================================================*/



/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/



/* FIXME: SBCPIO sbc_pio exists in sbc.c and has initialize() called in startup */
int cmd_test_pio(int argc, char *argv[])
{
	uint16_t value;
	SBCPIO_EXT &diag = sbc_pio.ext; /* hack */
	pioext_spi_interface pioext_spi(diag);
	mcp23s17 exp(pioext_spi);

	exp.initialize();
	exp.reset();



	printf("Starting PIO test:\n");

	printf("* Testing data bus.\n");

	for(int pass = 0; pass < 2; pass++)
	{
		for(int i = 0; i < 16; i++)
		{
			uint16_t expected = 1 << i;
			
			if(pass) 
			{
				expected = ~expected;
			}

			sbc_pio.set_data_pullup_level(expected);	
			uint16_t internal_actual = sbc_pio.read_data();
			uint16_t pio_actual = 0;

			pio_actual  = exp.read_reg(0, MCP23S17_Regs::GPIOA);
			pio_actual |= exp.read_reg(0, MCP23S17_Regs::GPIOB) << 8;

			printf("Wrote: %04X Internal:%04X PIO:%04X\n",
				expected,
				internal_actual,
				pio_actual
				);	
		}		
	}

	printf("* Testing address bus.\n");

	// configure dirs
	exp.write_reg(3, MCP23S17_Regs::IODIRA, 0x01); /* NOTE: Grounded GPA0 by accident ! */
	exp.write_reg(3, MCP23S17_Regs::IODIRB, 0x00);
	exp.write_reg(2, MCP23S17_Regs::IODIRA, 0x00);

	// write out pattern
	exp.write_reg(3, MCP23S17_Regs::OLATA, 0xCC); /* NOTE: Grounded GPA0 by accident ! */
	exp.write_reg(3, MCP23S17_Regs::OLATB, 0xBB);
	exp.write_reg(2, MCP23S17_Regs::OLATA, 0xAA);

	uint32_t addr = sbc_pio.read_address();
	printf("A:%08X\n", addr);

	

	exp.reset();
	

	printf("Test finished.\n");

	return 1;
}



/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/

int cmd_detect_expander(int argc, char *argv[])
{
	int count = 0;
	SBCPIO_EXT &diag = sbc_pio.ext;
	pioext_spi_interface pioext_spi(diag);
	mcp23s17 mcp(pioext_spi);

	mcp.initialize();
	mcp.reset();

	const uint8_t kCheckPattern = 0xA5;
	const uint8_t kCheckIndex = 0xC8;

	printf("Checking for presence of MCP23S17 devices:\n");

	/* Program patterns into each I/O expander */
	for(int device = 0; device < 8; device++)
	{
		int id = kCheckIndex + device;
		mcp.write_reg(device, GPPUA, kCheckPattern);
		mcp.write_reg(device, GPPUB, id);
		
	}

	/* Validate patterns */
	for(int device = 0; device < 8; device++)
	{
		int id = kCheckIndex + device;
		uint8_t result[2];

		result[0] = mcp.read_reg(device, GPPUA);
		result[1] = mcp.read_reg(device, GPPUB);

		if(result[0] == kCheckPattern && result[1] == id)
		{
			printf("- Detected device at address %d (got %02X,%02X).\n", device, result[0], result[1]);
			++count;
		}
		else
		{
			printf("- No device found at address %d (got %02X,%02X).\n", device, result[0], result[1]);		
		}
	}
	printf("- Found %d devices total.\n", count);
	printf("Detection finished.\n");
	mcp.reset();

	return 1;
}





int cmd_test_interface(int argc, char *argv[])
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


int cmd_test_pioram(int argc, char *argv[])
{
	bool error = false;

	printf("Testing PIO RAM ...\n");

	const uint8_t pattern[] = {0x00, 0x55, 0xAA, 0xFF, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

	SBCPIO pio;
	pio.initialize();

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
	SBCPIO_EXT &diag = sbc_pio.ext;
	pioext_spi_interface pioext_spi(diag);
	mcp23s17 mcp(pioext_spi);
	PIOTester tester(mcp);
	tester.initialize();


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
