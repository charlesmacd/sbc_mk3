

#if 1
#include <stdio.h> // sprintf
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "sbc.hpp"
#include "L1_Peripheral/uart.hpp"
#include "L3_Application/cli.hpp"
#include "newlib/mem_heap.hpp"
#include "cli_cmd.hpp"
#include "timer.hpp"


/* Main area base */
#define SBCPIO_BASE			0x900000

/* Sub area offset */
#define PIO_USB_BASE		(SBCPIO_BASE + 0x000000)
#define PIO_EXT_BASE		(SBCPIO_BASE + 0x004000)
#define PIO_CLK_BASE		(SBCPIO_BASE + 0x008000)
#define PIO_REG_BASE		(SBCPIO_BASE + 0x00C000)
#define PIO_RAM_BASE		(SBCPIO_BASE + 0x010000)

#define PIO_RAM_SIZE		0x8000 /* LSB of each word */

#define REG_ADL				(PIO_REG_BASE + 0x01)
#define REG_ADM				(PIO_REG_BASE + 0x03)
#define REG_ADH				(PIO_REG_BASE + 0x05)
#define REG_DBL				(PIO_REG_BASE + 0x07)
#define REG_DBH				(PIO_REG_BASE + 0x09)
#define REG_CTL				(PIO_REG_BASE + 0x0B)
#define REG_CTH				(PIO_REG_BASE + 0x0D)
#define REG_MISC			(PIO_REG_BASE + 0x0F)
#define REG_WAITREQ			(PIO_REG_BASE + 0x01)
#define REG_FFCK_CLR		(PIO_REG_BASE + 0x03)
#define REG_FFCK_PRE		(PIO_REG_BASE + 0x05)

volatile uint8_t  *PIO_USB =  (volatile uint8_t *)(PIO_USB_BASE);
volatile uint16_t  *PIO_EXT =  (volatile uint16_t *)(PIO_EXT_BASE);
volatile uint8_t  *PIO_CLK =  (volatile uint8_t *)(PIO_CLK_BASE);
volatile uint8_t  *PIO_REG =  (volatile uint8_t *)(PIO_REG_BASE);
volatile uint16_t *PIO_RAM = (volatile uint16_t *)(PIO_RAM_BASE);

/* Read */
volatile uint8_t *PIO_REG_ADL 	= (volatile uint8_t *)REG_ADL;
volatile uint8_t *PIO_REG_ADM 	= (volatile uint8_t *)REG_ADM;
volatile uint8_t *PIO_REG_ADH 	= (volatile uint8_t *)REG_ADH;

/* Write */
volatile uint8_t *PIO_REG_WAITREQ 	= (volatile uint8_t *)REG_WAITREQ;
volatile uint8_t *PIO_REG_CLK_CLR 	= (volatile uint8_t *)REG_FFCK_CLR;
volatile uint8_t *PIO_REG_CLK_PRE 	= (volatile uint8_t *)REG_FFCK_PRE;

/* Read/write */
volatile uint8_t *PIO_REG_DBL 	= (volatile uint8_t *)REG_DBL;
volatile uint8_t *PIO_REG_DBH 	= (volatile uint8_t *)REG_DBH;
volatile uint8_t *PIO_REG_CTL   = (volatile uint8_t *)REG_CTL;
volatile uint8_t *PIO_REG_CTH   = (volatile uint8_t *)REG_CTH;
volatile uint8_t *PIO_MISC 		= (volatile uint8_t *)REG_MISC; /* Connects to JP13, not used */


/* WAITREQ_LD */
#define B_WAITREQ			0
#define B_USER_LED			1

/* CTOL_LD */
#define B_TGT_PIO_DTACK		0
#define B_TGT_BGACK			1
#define B_TGT_BR			2
#define B_TGT_VPA			3
#define B_TGT_BERR			4
#define B_TGT_HALT			5
#define B_TGT_RESET			6
#define B_TPS_EN			7 /* DEPRECATED; now spare I/O */

#define F_TGT_PIO_DTACK		(1 << B_TGT_PIO_DTACK)
#define F_TGT_BGACK			(1 << B_TGT_BGACK)
#define F_TGT_BR			(1 << B_TGT_BR)
#define F_TGT_VPA			(1 << B_TGT_VPA)
#define F_TGT_BERR			(1 << B_TGT_BERR)
#define F_TGT_HALT			(1 << B_TGT_HALT)
#define F_TGT_RESET			(1 << B_TGT_RESET)
#define F_TPS_EN			(1 << B_TPS_EN)

/* CTOH_LD */
#define B_TGT_IPL2			0
#define B_TGT_IPL1			1
#define B_TGT_IPL0			2
#define B_TGT_CLK			3
#define B_CLKSEL0			4
#define B_CLKSEL1			5
#define B_OUT3				6 /* TGT_PU_LVL output ctrl */
#define B_OUT6				7 /* Secondary channel */

#define F_TGT_IPL2			(1 << B_TGT_IPL2)
#define F_TGT_IPL1			(1 << B_TGT_IPL1)
#define F_TGT_IPL0			(1 << B_TGT_IPL0)
#define F_TGT_CLK			(1 << B_TGT_CLK)
#define F_CLKSEL0			(1 << B_CLKSEL0)
#define F_CLKSEL1			(1 << B_CLKSEL1)
#define F_OUT3				(1 << B_OUT3)
#define F_OUT6				(1 << B_OUT6)

#define F_TGT_IPL_MASK		(F_TGT_IPL2 | F_TGT_IPL1 | F_TGT_IPL0)
#define F_CLKSEL_MASK		(F_CLKSEL1 | F_CLKSEL0)

#define F_STATUS_TPS_EN		0x10
#define F_STATUS_TPS_FLT	0x20
#define F_STATUS_FF_CKO		0x40
#define F_STATUS_WAITREQ	0x80
 
/* WAITREQ_LD# */


void dump_memory(uint32_t address, uint32_t size);

/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/


#if 0

#if 0

halt reset outputs work to control led
just that divider behavior makes low not 0v, but 1.6v

currently has pull-up to vcc, so all 68000 inputs will be '1'

halt is 5v high, 1.6v low

                  
                      +5V
                       |
                       E(10K RA)
            10K        |
574[x] ---- ///// -----+--------- <signal to target>

Sink case
Vo=2.5V


#endif


typedef struct __attribute__ ((packed))
{
	/* Address */
	struct __attribute__ ((packed))
	{
		uint8_t adl;
		uint8_t adm;
		uint8_t adh;
	} ab;

	struct __attribute__ ((packed))
	{
		uint8_t dbl;
		uint8_t dbh;
	} db;

	/* CTIL */
	uint8_t tfc:3;
	uint8_t te:1;
	uint8_t tvma:1;
	uint8_t thalt:1;
	uint8_t treset:1;
	uint8_t ctil_sparein:1;

	/* CTIH */
	uint8_t tas:1;
	uint8_t tuds:1;
	uint8_t tlds:1;
	uint8_t trw:1;
	uint8_t tbg:1;
	uint8_t ctih_sparein:3;

	uint8_t misc;

} target_state;

extern void target_read_state(target_state *p);


#if 0
	*PIO_REG_WAITREQ = 0x00;
	*PIO_REG_CTL = 0x00;
	*PIO_REG_CTH = 0x00;

	/*
	0: pio tclk from 574
	1: aux clk from ??? (jumpered to be fixed HIGH)
	2: ff_cko
	3: x1m
	*/

	target_state p;
	__asm__ __volatile__ ("reset");
	__asm__ __volatile__ ("reset");
	target_read_state(&p);
	__asm__ __volatile__ ("reset");
	__asm__ __volatile__ ("reset");

	*PIO_REG_CTH = (0 << B_CLKSEL1 | 0 << B_CLKSEL0 | 0 << B_TGT_CLK);
	readkey();// 5v
	*PIO_REG_CTH = (0 << B_CLKSEL1 | 0 << B_CLKSEL0 | 1 << B_TGT_CLK);	
	readkey();// 5v

	*PIO_REG_CTH = (0 << B_CLKSEL1 | 1 << B_CLKSEL0 | 0 << B_TGT_CLK);
	readkey();// 5v
	*PIO_REG_CTH = (1 << B_CLKSEL1 | 0 << B_CLKSEL0 | 0 << B_TGT_CLK);
	readkey();// 5v
	*PIO_REG_CTH = (1 << B_CLKSEL1 | 1 << B_CLKSEL0 | 0 << B_TGT_CLK);
	readkey(); // 2.4, but oscope can't pick it up??
	*PIO_REG_CTH = (0 << B_CLKSEL1 | 0 << B_CLKSEL0 | 0 << B_TGT_CLK);
	

#if 0
// correct, as tc4428 has inverting drivers
	// so in=H drives out low

// drives low, 0v
	*PIO_REG_CTH = (1 << B_OUT3 | 1 << B_OUT6);
	readkey();
	
// drives high, 5v
	*PIO_REG_CTH = (0 << B_OUT3 | 0 << B_OUT6);
	readkey();
#endif
#endif

uint8_t gcth;
uint8_t gctl;
uint8_t gwaitreq;

void target_set_power(bool enabled)
{
	uint8_t status = PIO_MISC[0];

	gwaitreq = 0x00;

	if(enabled)
		gwaitreq |= 0x02;
	else
		gwaitreq &= ~0x02;

	if(status & F_STATUS_WAITREQ)
		gwaitreq |= 0x01;

	PIO_REG_WAITREQ[0] = gwaitreq;	
}


void target_load_waitreq(bool state)
{
	uint8_t status = PIO_MISC[0];

	gwaitreq = 0x00;

	if(state & 1)
		gwaitreq |= 0x01;
	else
		gwaitreq &= ~0x01;

	if(status & F_STATUS_TPS_EN)
		gwaitreq |= 0x02;

	PIO_REG_WAITREQ[0] = gwaitreq;	
}


#define F_STATUS_TPS_EN		0x10
#define F_STATUS_TPS_FLT	0x20
#define F_STATUS_FF_CKO		0x40
#define F_STATUS_WAITREQ	0x80

void target2_set_clock_mode(uint8_t mode)
{
	gcth &= ~F_CLKSEL_MASK;

	if(mode & 1)
		gcth |= F_CLKSEL0;
	if(mode & 2)
		gcth |= F_CLKSEL1;

	PIO_REG_CTH[0] = gcth;
}

void target2_set_data(uint16_t data)
{
	PIO_REG_DBL[0] = (data >> 0) & 0xFF;
	PIO_REG_DBH[0] = (data >> 8) & 0xFF;
}

void target2_assert_reset(bool state)
{
	if(state)
	{
		gctl &= ~(F_TGT_HALT | F_TGT_RESET);
	}
	else
	{
		gctl |= (F_TGT_HALT | F_TGT_RESET);
	}

	PIO_REG_CTL[0] = gctl;
}

void target2_set_ipl(int level)
{
	gcth &= ~F_TGT_IPL_MASK;
	if(level & 0x04) gcth |= F_TGT_IPL2;
	if(level & 0x02) gcth |= F_TGT_IPL1;
	if(level & 0x01) gcth |= F_TGT_IPL0;
	PIO_REG_CTH[0] = gcth;	
}


int cmd_wiggle(int argc, char *argv[])
{
	volatile uint8_t *CTRL_OUT3 = (volatile uint8_t *)0xFFFF8087;

	while(1)
	{
		delay_ms(100);
		CTRL_OUT3[0] = 1;
		delay_ms(100);
		CTRL_OUT3[0] = 0;
	}
}

int cmd_pio(int argc, char *argv[])
{





#if 0
	uint8_t ctl;
	uint8_t cth;

	ctl = 0;
	ctl |= (1 << B_TGT_PIO_DTACK);
	ctl |= (1 << B_TGT_BGACK);
	ctl |= (1 << B_TGT_BR);
	ctl |= (1 << B_TGT_VPA);
	ctl |= (1 << B_TGT_BERR);
	ctl &= ~(1 << B_TGT_HALT | 1 << B_TGT_RESET);
	PIO_REG_CTL[0] = ctl;

	cth = 0;
	cth |= 
#define B_TGT_IPL2			0
#define B_TGT_IPL1			1
#define B_TGT_IPL0			2
#define B_TGT_CLK			3
#define B_CLKSEL0			4
#define B_CLKSEL1			5
#define B_OUT3				6 /* TGT_PU_LVL output ctrl */
#define B_OUT6				7 /* Secondary channel */
	PIO_REG_CTL[1] = cth;


	/* Select free-running clock mode */
	PIO_REG_CTL[0] = 0x80;
	PIO_REG_CTH[0] = (1 << B_CLKSEL1 | 1 << B_CLKSEL0 | 0 << B_TGT_CLK);

#endif

	gctl = 0;
	gcth = 0;
	gwaitreq = 0;

	gctl |= F_TGT_PIO_DTACK;
	gctl |= F_TGT_BGACK;
	gctl |= F_TGT_BR;
	gctl |= F_TGT_VPA;
	gctl |= F_TGT_BERR;
	gctl |= F_TPS_EN;

#if 1
	target_set_ipl(7);

	target_set_clock_mode(3);

	target_load_waitreq(0);

	target_set_data(0xABCE);

	target_set_power(true);
	target_assert_reset(true);

	for(int k = 0; k < 0x100; k++)
		(void)PIO_REG_DBL[0];

	target_assert_reset(false);
#endif

	printf("Running ... (L:%02X, H:%02X)\n", gctl, gcth);
	uart_readkey();
	uint8_t adl = PIO_REG_ADL[0] & 0xFE;
	uint8_t adm = PIO_REG_ADL[0];
	uint8_t adh = PIO_REG_ADL[0];
	uint8_t dbl = PIO_REG_DBL[0];
	uint8_t dbh = PIO_REG_DBH[0];

	uint32_t address = (adh << 16 | adm << 8 | adl);
	uint16_t data = (dbh << 8 | dbl);
	printf("TGT ADDR = %06X\n", address);
	printf("TGT DATA = %04X\n", data);

	printf("Shutting down...");	
	uart_readkey();
	target_set_power(false);

	printf("Test complete.\n");

	return 0;
}


#endif






int cmd_piodbus(int argc, char *argv[])
{
	*PIO_REG_WAITREQ = 0x00;
	*PIO_REG_CTL = 0x00;
	*PIO_REG_CTH = 0x00;

	bool error;

	error = false;
	printf("Testing DBUS readback ...\n");

	for(int i = 0; i < 0x10000; i++)
	{
		uint16_t key = i;
		PIO_REG_DBL[0] = ((key >> 8) & 0xFF);
		PIO_REG_DBH[0] = ((key << 0) & 0xFF);
		uint16_t temp = 0xDEAD;
		temp = PIO_REG_DBL[0] << 8;
		temp |= (temp & 0xFF00) | (PIO_REG_DBH[0] & 0x00FF);
		if(temp != key)	
		{
			error = true;
			printf("Test error: got %04X expected %04X\n", temp, key);
			break;
		}	
	}
	PIO_REG_DBL[0] = 0x00;
	PIO_REG_DBH[0] = 0x00;

	printf("DBUS readback test: %s\n", error ? "NG" : "OK");

	printf("Test complete.\n");

	return 0;
}






















int cmd_pioram(int argc, char *argv[])
{
	bool error = false;

	printf("Testing PIO RAM ...\n");

	const uint8_t pattern[] = {0x00, 0x55, 0xAA, 0xFF, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

	for(int loop = 0; loop < sizeof(pattern); loop++)
	{
		printf("Testing pattern: %02X (%d of %d)\n", pattern[loop], 1+loop, sizeof(pattern));

		for(int i = 0; i < PIO_RAM_SIZE; i++)
		{
			PIO_RAM[i] = pattern[loop];
		}

		for(int i = 0; i < PIO_RAM_SIZE; i++)
		{
			uint8_t expected = pattern[loop];
			uint8_t temp = (PIO_RAM[i] & 0x00FF);
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

#if 0

	volatile uint8_t *CS_CTRL = (volatile uint8_t *)0xFFFF8087;

ffff 8081 : q0 : pcf_reset#
ffff 8083 : q1 : od1#
ffff 8085 : q2 : od0#
ffff 8087 : q3 : spi_cs#
ffff 8089 : q4 : out3
ffff 808b : q5 : out2
ffff 808d : q6 : out1
ffff 808f : q7 : out0



#endif


int cmd_spi(int argc, char *argv[])
{
	volatile uint8_t *SPI_REG = (volatile uint8_t *)0xFFFF90C1;

	volatile uint8_t *SPI_CFG = (volatile uint8_t *)0xFFFF9081;
	volatile uint8_t *SPI_DMA = (volatile uint8_t *)0xFFFF80C1;


	volatile uint8_t *CS_CTRL = (volatile uint8_t *)0xFFFF8087;

	static volatile uint8_t *REG_STATUS 	= (volatile uint8_t *)(PIO_BASE  + 0x40);

	for(int i = 0; i < 0x10; i++)
	{
		SPI_REG[0] = i;
		delay_ms(1);
	}
	SPI_REG[0] = 0;

	// 1880ns / 16 = 117ns
	// at 8 mhz clock should be about 125ns so close enough



	CS_CTRL[0] = 1;
	SPI_CFG[0] = 0x00; // makes sclk go low, because cnt is reset from 1111 to 0000
	__asm__ __volatile__("nop");
	__asm__ __volatile__("nop");
	__asm__ __volatile__("nop");
	__asm__ __volatile__("nop");

	CS_CTRL[0] = 0;


	SPI_DMA[0] = 0xA5;
//	while(REG_STATUS[0] & 0x04)
//			;

//	SPI_CFG[0] = 0x00; // makes sclk go low, because cnt is reset from 1111 to 0000
	SPI_DMA[0] = 0xC3;
//	while(REG_STATUS[0] & 0x04)
//			;

//	SPI_CFG[0] = 0x00; // makes sclk go low, because cnt is reset from 1111 to 0000
	SPI_DMA[0] = 0xB2;
//	while(REG_STATUS[0] & 0x04)
//			;

	CS_CTRL[0] = 1;


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


extern cli_cmd_t terminal_cmds[];

static int cmd_list(int argc, char *argv[])
{	
	cli_cmd_t *cli_cmd_tab = terminal_cmds;

	printf("Available commands:\n");
	for(int i = 1; cli_cmd_tab[i].name != NULL; i++)
	{
		printf("- %s\n", cli_cmd_tab[i].name);
	}
}


int cmd_clrscr(int argc, char *argv[])
{
	printf(ANSI_HOME);
	printf(ANSI_CLRSCR);
}


#define TO_MHZ(x)			((x) / 1000000)

/* Display software information */
static int cmd_info(int argc, char *argv[])
{
	printf("Hardware:   SBC-V4\n");
	printf("Build date: %s\n", __DATE__);
	printf("Build time: %s\n", __TIME__);

	printf("CPU clock:         %d MHz\n", TO_MHZ(system_controller.get_cpu_clock()));
	printf("Peripheral clock:  %d MHz\n", TO_MHZ(system_controller.get_peripheral_clock()));

	return 0;
}


static int cmd_isr(int argc, char *argv[])
{
	uint32_t default_isr = *(uint32_t *)(0x100004);

	default_isr = 0x540;

	printf("List of non-default ISRs:\n");

	for(int i = 2; i < 256; i++)
	{
		uint32_t current_isr =  *(uint32_t *)(0x100002 + i * 8);

		if(current_isr != default_isr)
		{
			printf("Vector %03d : %08X\n", i, current_isr);
		}
	}
}

extern "C" {
    char _end;
	char *__stack;
	char *_text_start;
	char *_data_start;
	uint32_t _stext;
	uint32_t _sdata;
	uint32_t _sbss;
	
	uint32_t _bss_start;
	uint32_t _start;
	uint32_t _bend;
};

#if 0
  	_bss_start = .;
    _start = . ;
    *(.shbss)
    *(.bss .bss.*)
    *(COMMON)
    _bend = . ;
  } > ram
  _sbss = SIZEOF (.bss);
#endif

/* Display memory information */
static int cmd_mem(int argc, char *argv[])
{


	const uint32_t wram_start = 0x100000; 
	const uint32_t wram_size  = 0x100000;
	const uint32_t isrtable_size = 0x800;
	const uint32_t stack_bottom = wram_start + isrtable_size;

	uint32_t heap_size = (wram_start + wram_size - (uint32_t)sbrk(0));
	uint32_t stack_remaining = get_sp() - stack_bottom;

	printf("Stack and heap information:\n");
	printf("* Heap start:      %08X\n", sbrk(0));
	printf("* Heap size:       %08X (%dk)\n", heap_size, heap_size >> 10);
	printf("* Stack pointer:   %08X\n", get_sp());
	printf("* Stack remaining: %08X (%dk)\n", stack_remaining, stack_remaining >> 10);
	printf("\n");

	printf("Loaded program information:\n");
	printf("* .text   : %08X-%08X (%08X) (%dk)\n",
		(uint32_t)&_text_start,
		(uint32_t)&_text_start + (uint32_t)&_stext - 1,
		(uint32_t)&_stext,
		(uint32_t)&_stext >> 10
		); 
	printf("* .data   : %08X-%08X (%08X) (%dk)\n",
		(uint32_t)&_data_start,
		(uint32_t)&_data_start + (uint32_t)&_sdata - 1,
		(uint32_t)&_sdata,
		(uint32_t)&_sdata >> 10
		);
	printf("* .bss    : %08X-%08X (%08X) (%dk)\n",
		(uint32_t)&_bss_start,
		(uint32_t)&_bss_start + (uint32_t)&_sbss - 1,
		(uint32_t)&_sbss,
		(uint32_t)&_sbss >> 10
		);
	printf("\n");

	printf("Symbols:\n");
	printf("* _text_start : %08X\n", (uint32_t)&_text_start);
	printf("* _stext      : %08X\n", (uint32_t)&_stext);
	printf("* _data_start : %08X\n", (uint32_t)&_data_start);
	printf("* _sdata      : %08X\n", (uint32_t)&_sdata);
	printf("* _bss_start  : %08X\n", (uint32_t)&_bss_start);
	printf("* _bss_end    : %08X\n", (uint32_t)&_bss_start);
	printf("* _sbss       : %08X\n", (uint32_t)&_sbss);
	printf("* _end        : %08X\n", (uint32_t)&_end);
	printf("* __stack     : %08X\n", (uint32_t)&__stack);
	printf("BSS section:\n");
	printf("* _bss_start  : %08X\n", (uint32_t)&_bss_start);
	printf("* _bss_end    : %08X\n", (uint32_t)&_start);
	printf("* _bss_start  : %08X\n", (uint32_t)&_bend);
	printf("* _bss_end    : %08X\n", (uint32_t)&_sbss);

	return 0;
}





/* Restart FLASH boot loader */
static int cmd_reboot(int argc, char *argv[])
{
	printf("Returning to FLASH bootloader ...\n");
	soft_reboot();
	return 0;
}


/* Restart RAM program */
static int cmd_reset(int argc, char *argv[])
{
	printf("Restarting loaded RAM program ...\n");
	soft_reset();
	return 0;
}


/* Dump memory */
static int cmd_memdump(int argc, char *argv[])
{
	const int width = 0x10;
	uint32_t address = 0;
	int nargs = argc - 1;

	if(nargs == 0)
		address = 0;
	else
		address = strtol(argv[1], NULL, 16);

	printf("Address: %X\n", address);

	dump_memory(address, 0x100);
	return 0;
}






static int cmd_flash(int argc, char *argv[])
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





/* Set POST display */
static int cmd_post(int argc, char *argv[])
{
	if(argc == 2)
	{
		uint16_t value = strtoul(argv[1], NULL, 16);
		system_controller.set_post(value);
		system_controller.set_post_dp(value >> 8);
	}
	else
	{
		printf("usage: %s <value>\n", argv[0]);
	}

	return 0;
}


 
/* Control user LED */
static int cmd_led(int argc, char *argv[])
{
	volatile uint16_t *REG_USER_LED  = (volatile uint16_t *)0xFFFF8046;

	if(argc == 2)
	{
		if(strcmp(argv[1], "on") == 0)
		{
			REG_USER_LED[0] = 0;
		}
		else
		{
			REG_USER_LED[0] = 1;
		}
	}
	else
	{
		printf("usage: %s <on|off>\n", argv[0]);
	}


	return 0;
}


#if 0
extern uint32_t pic_flash_library_base;

void (*pic_flash_write)(uint32_t address, uint32_t num_pages, uint8_t *data) =  (void *)(&pic_flash_library_base + 0x00);
void (*pic_flash_read)(uint32_t address, uint32_t num_pages, uint8_t *data)  =  (void *)(&pic_flash_library_base + 0x01);
#endif
static int cmd_debug(int argc, char *argv[])
{
	uint8_t *buffer = (uint8_t *)0x104000;

	for(int i = 0; i < 256; i++)
			buffer[i] = 0x80 ^ i;

//	pic_flash_write(0x200000, 0x80, buffer);

//	pic_flash_read(0x200000, 0x80, buffer);

	dump_memory(0x104000, 0x100);
}


int  cmd_alloc(int argc, char *argv[])
{
	printf("OLD: SBRK:%08X, SP:%08X\n", sbrk(0), get_sp());

	if(argc == 2)
	{
		uint32_t value = strtoul(argv[1], NULL, 16);
		void *p = sbrk(value);
		printf("PTR: %08X\n", p);
	}
	else
	{
		printf("usage: %s <value>\n", argv[0]);
	}

	printf("NEW: SBRK:%08X, SP:%08X\n", sbrk(0), get_sp());
	return 0;

}

int cmd_iotest(int argc, char *argv[])
{
	volatile uint16_t *OUT = (volatile uint16_t *)0xFFFF8080;
	printf("Now toggling:\n");
	bool toggling = true;
	while(toggling)
	{
		if(uart.read_blocking())
		{
			toggling = false;
		}
		delay_ms(10);
//		OUT[4] = 1;
		delay_ms(10);
//		OUT[4] = 0;
	}
	printf("Test finished.\n");
}










int cmd_timer(int argc, char *argv[])
{
	bool running = true;

	printf("Starting timer test ...\n");
	printf("SR = %04X\n", get_sr());

	__systick_flag = 0;
	__interval_1us_flag = 0;
	__interval_1ms_flag = 0;

	uint32_t systick_count = 0;
	uint32_t interval_1ms_count = 0;
	uint32_t interval_1us_count = 0;
	uint32_t capture_1ms = 0xdeadbeef;
	uint32_t capture_1us = 0xdeadbeef;

	int interval_1ms = 500; 			// 500ms
	int interval_1us = 500 * 1000;		// 500,000us

	printf("Set 1ms timer to %d ticks.\n", interval_1ms);
	set_1ms_timer(interval_1ms);	
	
	printf("Set 1us timer to %d ticks.\n", interval_1us);
	set_1us_timer(interval_1us);	

	while(running)
	{
		if(uart.keypressed())
		{
			running = false;
			printf("Status: User requested exit.\n");
		}

		if(__interval_1ms_flag)
		{
			__interval_1ms_flag = 0;
			++interval_1ms_count;
			capture_1ms = systick_count;
			printf("Status: 1ms timer expired.\n");
		}
		if(__interval_1us_flag)
		{
			__interval_1us_flag = 0;
			++interval_1us_count;
			capture_1us = systick_count;
			printf("Status: 1us timer expired.\n");
		}
		if(__systick_flag)
		{
			__systick_flag = 0;
			++systick_count;
		}
	}

	printf("Timer test complete.\n");
	printf("- Measured %08X system ticks.\n", systick_count);
	printf("- Measured %08X millisecond ticks.\n", interval_1ms_count);
	printf("- Measured %08X microsecond ticks.\n", interval_1us_count);
	printf("- Captured systick by 1ms timer = %08X\n", capture_1ms);
	printf("- Captured systick by 1us timer = %08X\n", capture_1us);
}





int cmd_probeout(int argc, char *argv[])
{
	uint8_t ctol;
	uint8_t ctoh;

	ctol = 0;
	ctoh = 0;
	printf("Running probe out test.\n");
	printf("Press any key to stop.\n");
	int count = 0;
	bool running = true;
	while(running)
	{
		++count;
		if(uart.read_blocking())
			running = false;
		delay_ms(250);

		int v = count >> 2;

		ctol = 0;

		if(v & 1)
			ctol |= (1 << B_TGT_RESET);
		if(v & 2)
			ctol |= (1 << B_TGT_HALT);

		PIO_REG_CTL[0] = ctol;
		PIO_REG_CTH[0] = ctoh;

	}
	printf("Test complete.\n");
}




int cmd_probein(int argc, char *argv[])
{
	/* OUT3 */
	volatile uint8_t *PROBE = (volatile uint8_t *)0xFFFF8089;

	// 4.4k didn't work
	// 2k does
	// remember there's 4.7k p/u

	printf("NOTE: Attached OUT3 on SBC through series resistor (1K or lower) to probe clip.\n");

	uint16_t dbus;
	uint32_t abus;
	bool running = true;
	int count = 0;
	while(running)
	{
		if(uart.read_blocking())
			running = false;

		PROBE[0] = (count++ >> 2) & 1;

		abus = 0;
		abus |= PIO_REG_ADH[0] << 16;
		abus |= PIO_REG_ADM[0] << 8;
		abus |= PIO_REG_ADL[0] << 0;
		abus &= 0x00FFFFFE;
		
		dbus = 0;
		dbus |= PIO_REG_DBH[0] << 8;
		dbus |= PIO_REG_DBL[0] << 0;

		uint8_t ctil, ctih;

		ctil = PIO_REG_CTL[0];
		ctih = PIO_REG_CTH[0];

		// probe all inputs works
		// probe rst/hlt makes leds change, but not readback ???
		// treset#,thalt# go to uln2003an directly
		// and 245 to input
		// 
		// but directly with no series resistor it works
		// e.g. hardware drives treset# high, we pull low with 2k
		// voltage divider output is still not sufficient to reach threshold

		printf("%s", ANSI_HOME);
		printf("D:%04X A:%06X\n", dbus, abus);

		printf("FC:%d E:%d VMA:%d HLT:%d RST:%d\n",
			(ctil >> 0) & 7,
			(ctil >> 3) & 1,
			(ctil >> 4) & 1,
			(ctil >> 5) & 1,
			(ctil >> 6) & 1
			);

		printf("AS:%d UDS:%d LDS:%d R/W:%d BG:%d\n",
			(ctih >> 0) & 1,
			(ctih >> 1) & 1,
			(ctih >> 2) & 1,
			(ctih >> 3) & 1,
			(ctih >> 4) & 1
			);

		delay_ms(250);
	}

}



typedef struct
{
	/* Registers */
	uint8_t waitreq;
	uint8_t ctol;
	uint8_t ctoh;
	uint8_t dbol;
	uint8_t dboh;

	/* Inputs */
	uint32_t abus;
	uint16_t dbus;

	uint8_t fc:3;
	uint8_t e:1;
	uint8_t vma:1;
	uint8_t halt:1;
	uint8_t reset:1;

	uint8_t as:1;
	uint8_t uds:1;
	uint8_t lds:1;
	uint8_t rw:1;
	uint8_t bg:1;


	uint8_t zint:1;
	uint8_t znmi:1;
	uint8_t zhalt:1;
	uint8_t zmreq:1;
	uint8_t ziorq:1;
	uint8_t zwr:1;
	uint8_t zrd:1;
	uint8_t zbusack:1;
	uint8_t zwait:1;
	uint8_t zbusreq:1;
	uint8_t zreset:1;
	uint8_t zm1:1;
	uint8_t zrfsh:1;
	uint8_t bus_state;
	uint16_t dbus_rx;

} target_state_t;

target_state_t state;

void target_set_ipl(uint8_t level);
void target_flush_state(void);
void target_set_clock_mode(uint8_t mode);
void target_set_dbus(uint16_t value);
void target_reset(bool reset_en, bool halt_en);
void target_init(void);


#define TGT_CLK_MODE_PIO		0
#define TGT_CLK_MODE_AUX		1
#define TGT_CLK_MODE_FFCK		2
#define TGT_CLK_MODE_FRT		3


void target_flush_state(void)
{
	PIO_REG_CTL[0] = state.ctol;
	PIO_REG_CTH[0] = state.ctoh;
	PIO_REG_DBL[0] = state.dbol;
	PIO_REG_DBH[0] = state.dboh;
}

// waitreq_ld
// md1 is user_led# -> tps_en#
// pre= null
// clr= sysrst

// waitrq_ld
// md0 is waitreq node
// pre=taspre
// clr=piorst#

// misc oe -- from pio read decode y7 (ma[3:1] = 111)
// d7 = waitreq#
// d4 = user_led#

uint8_t target_read_misc(uint8_t position)
{
	uint8_t value = PIO_MISC[0];
	return (value & (1 << position)) ? 1 : 0;
}

uint8_t target_read_ctil(uint8_t position)
{
	uint8_t value = PIO_REG_CTL[0];
	return (value & (1 << position)) ? 1 : 0;
}

uint8_t target_read_ctih(uint8_t position)
{
	uint8_t value = PIO_REG_CTH[0];
	return (value & (1 << position)) ? 1 : 0;
}



void target_set_clock_mode(uint8_t mode)
{
	state.ctoh &= ~0x30;
	state.ctoh |= (mode & 0x03) << 4;
	target_flush_state();
}

void target_set_dbus(uint16_t value)
{
	state.dbol = (value >> 0) & 0xFF;
	state.dboh = (value >> 8) & 0xFF;
	target_flush_state();
}

void target_change_ctol(uint8_t position, uint8_t level)
{
	state.ctol &= ~(1 << position);
	if(level)
	{
		state.ctol |= (1 << position);
	}	
}
void target_change_ctoh(uint8_t position, uint8_t level)
{
	state.ctoh &= ~(1 << position);
	if(level)
	{
		state.ctoh |= (1 << position);
	}	
}








/*==========================================*/
/*==========================================*/
/*==========================================*/
/*==========================================*/
/*==========================================*/
/*==========================================*/


void target_set_ipl(uint8_t level)
{
	state.ctoh &= ~0x07;
	if(level & 0x01)
		state.ctoh |= 0x04;
	if(level & 0x02)
		state.ctoh |= 0x02;
	if(level & 0x04)
		state.ctoh |= 0x01;
	target_flush_state();
}


void target_set_pio_clk(uint8_t level)
{
	target_change_ctoh(3, level);
	target_flush_state();
}



/*==========================================*/
/*==========================================*/
/*==========================================*/


void target_set_pio_dtack(uint8_t level)
{
	target_change_ctol(0, level);
	target_flush_state();
}
void target_set_bgack(uint8_t level)
{
	target_change_ctol(1, level);
	target_flush_state();	
}
void target_set_br(uint8_t level)
{
	target_change_ctol(2, level);
	target_flush_state();	
}

void target_set_vpa(uint8_t level)
{
	target_change_ctol(3, level);
	target_flush_state();	
}

void target_set_berr(uint8_t level)
{
	target_change_ctol(4, level);
	target_flush_state();	
}

void target_set_reset(uint8_t level)
{
	state.zreset = level;
	target_change_ctol(6, level);
	target_flush_state();	
}

void target_set_halt(uint8_t level)
{
	target_change_ctol(5, level);
	target_flush_state();	
}

/*==========================================*/
/*==========================================*/
/*==========================================*/

#define MODE_Z80		1

void target_read_state(void)
{
	uint8_t ctil = PIO_REG_CTL[0];
	uint8_t ctih = PIO_REG_CTH[0];

	

	state.dbus = PIO_REG_DBH[0] << 8 | PIO_REG_DBL[0];


	state.abus = PIO_REG_ADH[0] << 16 | PIO_REG_ADM[0] << 8 | PIO_REG_ADL[0];
	state.abus &= 0x00FFFFFE;

	state.as = (ctih >> 0) & 1;
	state.uds = (ctih >> 1) & 1;
	state.lds = (ctih >> 2) & 1;
	state.rw = (ctih >> 3) & 1;
	state.bg = (ctih >> 4) & 1;

	state.fc = (ctil >> 0) & 7;
	state.e = (ctih >> 3) & 1;
	state.vma = (ctil >> 4) & 1;
	state.halt = (ctil >> 5) & 1;
	state.reset = (ctil >> 6) & 1;

#if MODE_Z80

	state.zhalt = (state.abus >> 7) & 1;
	state.zrfsh = (state.abus >> 6) & 1;
	state.zm1   = (state.abus >> 5) & 1;
	state.zwr   = (state.abus >> 4) & 1;
	state.zrd   = (state.abus >> 3) & 1;
	state.ziorq = (state.abus >> 2) & 1;
	state.zmreq = (state.abus >> 1) & 1;

	state.zbusack = state.bg;

	state.abus = (PIO_REG_ADH[0] << 8 | PIO_REG_ADM[0]) & 0xffff;
	state.dbus = PIO_REG_DBL[0];

	state.bus_state = state.zrd << 5 | state.zwr << 4 | state.zm1 << 3 | state.zrfsh << 2 | state.ziorq << 1 | state.zmreq;


#endif	
}

void target_set_power(uint8_t level)
{
	uint8_t waitreq = target_read_misc(7);
	uint8_t value = 0;
	
	if(waitreq)
		value |= 1;
	if(level)
		value |= 2;

	*PIO_REG_WAITREQ = value;
}

void target_set_waitreq(uint8_t level)
{
	uint8_t user_led = target_read_misc(4);
	uint8_t value = 0;
	
	if(level)
		value |= 1;
	if(user_led)
		value |= 2;

	*PIO_REG_WAITREQ = value;
}




void target_init(void)
{
	memset(&state, 0, sizeof(target_state_t));
	
	state.ctol = 0;
	state.ctoh = 0;

	target_set_pio_dtack(1);
	target_set_bgack(1);
	target_set_br(1);
	target_set_vpa(1);
	target_set_vpa(1);
	target_set_berr(1);

	target_set_dbus(0x7000);
	target_set_ipl(7);
	target_set_clock_mode(TGT_CLK_MODE_FRT);
	target_flush_state();


}	


bool wait_request(void)
{
	const uint32_t timeout_ms = 1000; // 1 second

	uint8_t state_waitreq;
	uint8_t state_reset;
	uint8_t state_halt;

	/* Record current time */
	uint32_t current_ms = __systick_count;

	while(1)
	{
		state_reset = target_read_ctil(6);
		state_halt = target_read_ctil(5);

		/* Halt state */
		if(state_reset == 1 && state_halt == 0)
		{
			return false;
		}

		/* Check if WAITREQ F/F is set */
		uint8_t state_waitreq = target_read_misc(7);
		if(state_waitreq == 1)
		{
			return true;
		}

		/* Check for time-out condition */
		uint32_t elapsed_ms = __systick_count - current_ms;
		if(elapsed_ms >= timeout_ms)
		{
			printf("ERROR: wait_request(): Timeout (%d ms elapsed)\n", timeout_ms);
			return false;
		}
	}
	return false;
}

const char *str_hr[] =
{
	"EXT.RST",
	"RST.INS",
	"DBL.HLT",
	"RUNNING",
};

const char *str_fc[] =
{
	"????", "UDAT", "UPRG", "????",
	"????", "SDAT", "SPRG", "IACK"
};



const char *size_tab[] =
{
	".W", ".B", ".B", ".X"
};

uint16_t bus_emulator_read(uint8_t fc, uint32_t address, uint8_t width)
{
	uint16_t data = 0xffff;
	uint8_t size = state.uds << 1 | state.lds;

	if(address < 0x020000)
	{
//		data = shinobi_prog[(address >> 1) & 0x0FFFF];
	}

	printf("R: FC:%d A:%06X DI:%04X S:%s\n", fc, address, data, size_tab[size]);
	return data;
}

void bus_emulator_write(uint8_t fc, uint32_t address, uint8_t width, uint16_t data)
{
	uint8_t size = state.uds << 1 | state.lds;
	printf("W: FC:%d A:%06X DO:%04X S:%s\n", 
		fc, 
		address, 
		data, 
		size_tab[size]
		);	
}


//uint16_t program




void target_set_nmi(uint8_t level)
{
	if(level)
	{
		target_set_ipl(7);
	}
	else
	{
		target_set_ipl(5);
	}
}

void target_set_int(uint8_t level)
{
	if(level)
	{
		target_set_ipl(7);
	}
	else
	{
		target_set_ipl(6);
	}
}



int cmd_zdiag(int argc, char *argv[])
{
	target_init();

	printf("Z-Board diagnostics.\n");
	printf("Ensure no devices are plugged into the target device sockets.\n");
	printf("1 : Power test.\n");
	printf("2 : Reset test.\n");
	printf("Press test number to continue ...\n");

	char ch = uart.read_blocking();
	switch(ch)
	{
		case '1':
			printf("- Power ON test.\n");
			target_set_power(1);
			printf("- Verify power LED, then press any key to continue.\n");
			uart.read_blocking();
			
			printf("- Power OFF test.\n");
			target_set_power(0);
			printf("- Verify power LED, then press any key to continue.\n");
			uart.read_blocking();
			break;

		case '2':
			printf("- Reset pulse test.\n");
			printf("- Set analyzer to capture on TRESET# and press any key to continue.\n");
			printf("- Observe pulse of H-L-H of 20ms each.\n");
			uart.read_blocking();
			target_set_reset(1);
			delay_ms(20);
			target_set_reset(0);
			delay_ms(20);
			target_set_reset(1);
			delay_ms(20);
			break;

		case '3':
			break;

		default:
			printf("Error: Unknown test `%c'.\n", ch);
			break;
	}


	printf("Diagnostics complete.\n");
}

/* crt0.s */
extern "C" void target_pulse_ffck(uint16_t clocks);

/* Z80 only */
void target_power_up_reset(void)
{
	/* Turn off power */
	target_set_power(0);

	/* Let power switch output settle */
	delay_ms(250);

	/* Pull ZRESET# low */
	target_set_reset(0);

	/* Turn on power */
	target_set_power(1);
	delay_ms(250);

	/* Clock device */
	target_pulse_ffck(128);

	/* Pull ZRESET# high */
	target_set_reset(1);
	delay_ms(10);
}

enum
{
	FFCK_POLARITY_ACTIVE_LOW,
	FFCK_POLARITY_ACTIVE_HIGH
};

void target_set_ffck_level(uint8_t level)
{
	if(level)
	{
		PIO_REG_CLK_PRE[0] = 0;
	}
	else
	{		
		PIO_REG_CLK_CLR[0] = 0;
	}

}

/* Set FFCK to level */
void target_set_ffck_polarity(uint8_t type)
{
	if(type == FFCK_POLARITY_ACTIVE_LOW)
	{
		/* Set CLK high so it can be pulsed low */
		PIO_REG_CLK_PRE[0] = 0;
	}
	else
	{
		/* Set CLK low so it can be pulsed high */
		PIO_REG_CLK_CLR[0] = 0;
	}
}

/* NOTE: PLD inverts TDTACK# to drive ZWAIT# 
   - Set TDTACK#=0 to drive ZWAIT#=H (running)
   - Set TDTACK#=1 to drive ZWAIT#=L (waiting)
*/
void target_set_pio_wait(uint8_t level)
{
	if(level)
	{
		target_set_pio_dtack(0);
	}
	else
	{
		target_set_pio_dtack(1);
	}
}

// tbr# -> busreq# : ctol bit 2
// tbg# -> busack  : ctih bit 4

uint8_t target_get_busack(void)
{
	return target_read_ctih(4);
}

void target_set_busreq(uint8_t level)
{
	target_change_ctol(2, level);
	target_flush_state();	
}

void target_set_wbase(uint8_t level)
{
	target_change_ctol(3, level);
	target_flush_state();	
}


#define BUS_RFSH			0b111010
#define BUS_RFSH_IDLE		0b111011
#define BUS_MEM_IDLE		0b111110
#define BUS_MEM_RDOP		0b010110
#define BUS_MEM_RDOP_IDLE	0b110111
#define BUS_MEM_READ		0b011110
#define BUS_MEM_WRITE		0b101110
#define BUS_IO_READ			0b011101
#define BUS_IO_WRITE		0b101101
#define BUS_IDLE			0b111111

const char *get_bus_name(uint8_t bus_state)
{
	switch(bus_state)
	{
		case BUS_RFSH: 
			return "RFSH";
		case BUS_RFSH_IDLE:
			return "RFSH IDLE";
		case BUS_MEM_IDLE:
			return "MEM IDLE";
		case BUS_MEM_RDOP:
			return "RDOP";
		case BUS_MEM_RDOP_IDLE:
			return "RDOP IDLE";
		case BUS_MEM_READ:
			return "RDMEM";
		case BUS_MEM_WRITE:
			return "WRMEM";
		case BUS_IO_READ:
			return "IOREAD";
		case BUS_IO_WRITE:
			return "IOWRITE";
		case BUS_IDLE:
			return "IDLE";
		default:
			return "????";
	}
}

bool is_bus_idle(uint8_t bus_state)
{
	switch(bus_state)
	{
		case BUS_MEM_IDLE:
		case BUS_RFSH_IDLE:
		case BUS_MEM_RDOP_IDLE:
		case BUS_IDLE:
			return true;
		default:
			return false;
	}
}



extern uint8_t test_data[0x10000];


void format_target_state(int index, char *buf)
{
	char fragment[128];

	buf[0] = 0;
	
	sprintf(fragment, "[%06X] IO:%d MR:%d M1:%d RF:%d W:%d R:%d HLT:%d RST:%d BA:%d ",
		index,
		state.ziorq,
		state.zmreq,
		state.zm1,
		state.zrfsh,
		state.zwr,
		state.zrd,
		state.zhalt,
		state.zreset,
		state.zbusack
		);
	strcat(buf, fragment);

	sprintf(fragment, "[A:%04X D:%02X P:%02X] ", state.abus, state.dbus, state.dbus_rx);
	strcat(buf, fragment);

	sprintf(fragment, "%s ", get_bus_name(state.bus_state));
	strcat(buf, fragment);

	if(state.bus_state == BUS_MEM_RDOP)
	{
		sprintf(fragment, "%02X", state.dbus_rx);	
		strcat(buf, fragment);
	}
}





int cmd_boot(int argc, char *argv[])
{
	target_init();
	target_set_int(1); /* TODO: INT and NMI override each other, read state and modify bits */
	target_set_nmi(1);
	target_set_busreq(1);
	target_set_wbase(1);
	target_set_pio_wait(1);
	target_set_clock_mode(TGT_CLK_MODE_FFCK);
	target_set_ffck_polarity(FFCK_POLARITY_ACTIVE_LOW);
	target_power_up_reset();

	/* Set initial data bus state */
	state.dbus_rx = 0x00;
	target_set_dbus(state.dbus_rx);

	/* Switch from FFCK to FRT mode */
	target_set_clock_mode(TGT_CLK_MODE_FRT);

	printf("FRT mode: Starting exec loop.\n");

	for(int i = 0; i < 20; i++)
	{
		/* Wait for Z80 bus state that triggered the WSG capture circuit */
		wait_request();

		/* Read current state */
		target_read_state();

		bool log = true;

		if(state.abus >= 0xe000 && state.abus <= 0xe7ff)
			log = false;
		if(state.abus >= 0xe800 && state.abus <= 0xe8ff)
			log = false;

		log = true;

		if(log)
		{	
			char buf[128];
			format_target_state(i, buf);
			printf("%s\n", buf);
		}

		/* Terminate current cycle with DTACK# */
		target_set_waitreq(0);
	}

	printf("FRT test complete.\n");
}




int cmd_step(int argc, char *argv[])
{
	uint32_t ncycles = 32;

	/* Check if user specified cycle count */
	if(argc == 2)
	{
		ncycles = atoi(argv[1]);
		printf("User set cycle count to %d\n", ncycles);
	}

	target_init();
	target_set_int(1); /* TODO: INT and NMI override each other, read state and modify bits */
	target_set_nmi(1);
	target_set_busreq(1);
	target_set_wbase(1);
	target_set_pio_wait(1);
	target_set_clock_mode(TGT_CLK_MODE_FFCK);
	target_set_ffck_polarity(FFCK_POLARITY_ACTIVE_LOW);
	state.dbus_rx = 0x76;
	target_set_dbus(state.dbus_rx);
	target_power_up_reset();

	printf("Step test.\n");

	for(int i = 0; i < ncycles; i++)
	{
		bool log = true;
		target_read_state();

		if(is_bus_idle(state.bus_state))
			log = false;

		// don't log spriteram at E800-E8FF
		if(state.abus >= 0xe800 && state.abus <= 0xe8ff)
			log = false;

/*
	Reset behavior
	Test 1
	- During execution of HALT instruction, assert RESET for one cycle
	- Opcode instruction finishes
	- There's a single cycle later of idle bus state where address is FFFF
	- 
*/			
		log = true;
		if(log)
		{
			char buf[128];
			format_target_state(i, buf);
			printf("%s\n", buf);
		}
		target_set_dbus(state.dbus_rx);
		target_pulse_ffck(1);
	}
	
	printf("End of step test.\n");
	return 0;
}








int cmd_edge(int argc, char *argv[])
{
	uint32_t ncycles = 32;

	/* Check if user specified cycle count */
	if(argc == 2)
	{
		ncycles = atoi(argv[1]);
		printf("User set cycle count to %d\n", ncycles);
	}

	target_init();
	target_set_int(1); /* TODO: INT and NMI override each other, read state and modify bits */
	target_set_nmi(1);
	target_set_busreq(1);
	target_set_wbase(1);
	target_set_pio_wait(1);
	target_set_clock_mode(TGT_CLK_MODE_FFCK);
	target_set_ffck_polarity(FFCK_POLARITY_ACTIVE_LOW);
	state.dbus_rx = 0xE3;
	target_set_dbus(state.dbus_rx);
	target_power_up_reset();

	uint8_t state_clock = 1;

	printf("Step test.\n");

	for(int i = 0; i < ncycles; i++)
	{
		bool log = true;
		target_read_state();

		if(is_bus_idle(state.bus_state))
			log = false;

		// don't log spriteram at E800-E8FF
		if(state.abus >= 0xe800 && state.abus <= 0xe8ff)
			log = false;

/*
	Reset behavior
	Test 1
	- During execution of HALT instruction, assert RESET for one cycle
	- Opcode instruction finishes
	- There's a single cycle later of idle bus state where address is FFFF
	- 
*/			
		log = true;
		if(log)
		{
			char buf[128];
			format_target_state(i, buf);
			printf("%s\n", buf);
		}
		target_set_dbus(state.dbus_rx);

		int point = 99;
#if 1
		if(i == point)
		{
			target_set_reset(0);
		}
#endif

		target_set_ffck_level(state_clock ^ 1);
		target_set_ffck_level(state_clock);
#if 1
		if(i == point)
		{
			target_set_reset(1);
		}

#endif


	}
	
	printf("End of step test.\n");
	return 0;
}




/* FRT NMI */
int cmd_nmi(int argc, char *argv[])
{
	printf("FRT NMI testing.\n");

	target_init();
	target_set_int(1); /* TODO: INT and NMI override each other, read state and modify bits */
	target_set_nmi(1);
	target_set_busreq(1);
	target_set_wbase(1);
	target_set_pio_wait(1);
	target_set_clock_mode(TGT_CLK_MODE_FFCK);
	target_set_ffck_polarity(FFCK_POLARITY_ACTIVE_LOW);
	target_power_up_reset();

	/* Set initial data bus state */
	state.dbus_rx = 0x00;
	target_set_dbus(state.dbus_rx);

	/* Switch from FFCK to FRT mode */
	target_set_clock_mode(TGT_CLK_MODE_FRT);

	printf("FRT NMI mode: Starting exec loop.\n");


	/* Request initial NMI */
	target_set_nmi(0);

/*
	NMI considerations
	- No feedback from system until stack writes reach external memory area
	- Is NMI sampled on clock edge or does it internally assert FF?
*/	

	for(int i = 0; i < 20; i++)
	{
		/* Wait for Z80 bus state that triggered the WSG capture circuit */
		wait_request();

		/* Read current state */
		target_read_state();
#if 1
		if(state.bus_state == BUS_MEM_RDOP)
		{
			target_set_nmi(0);
		}
		else
		{
			target_set_nmi(1);
		
		}
#endif
		bool log = true;
		if(log)
		{	
			char buf[128];
			format_target_state(i, buf);
			printf("%s\n", buf);
		}

		/* Terminate current cycle with DTACK# */
		target_set_waitreq(0);
	}

	printf("FRT NMI test complete.\n");


	return 0;
}


/*------------------------------------------------------*/

int cmd_int(int argc, char *argv[])
{
	printf("Interrupt test.\n");
	uint32_t total_cycles = 40;
	uint32_t trigger_cycle = 20;

	/* Check if user specified cycle count */
	if(argc == 2)
	{
		trigger_cycle = atoi(argv[1]);
		printf("Trigger cycle = %d\n", trigger_cycle);
	}

	target_init();
	target_set_int(1); /* TODO: INT and NMI override each other, read state and modify bits */
	target_set_nmi(1);
	target_set_busreq(1);
	target_set_wbase(1);
	target_set_pio_wait(1);
	target_set_clock_mode(TGT_CLK_MODE_FFCK);
	target_set_ffck_polarity(FFCK_POLARITY_ACTIVE_LOW);
	state.dbus_rx = 0x00;
	target_set_dbus(state.dbus_rx);
	target_power_up_reset();

	printf("Start of interrupt test.\n");

	for(int i = 0; i < total_cycles; i++)
	{
		bool log = true;
		target_read_state();

		if(is_bus_idle(state.bus_state))
			log = false;

		log = true;
		if(log)
		{
			char buf[128];
			format_target_state(i, buf);
			printf("%s\n", buf);
		}

		target_set_dbus(state.dbus_rx);

		
		if(i == trigger_cycle)
		{
			printf("NMI pulse.\n");

			target_set_ffck_level(0);
			delay_ms(250);

			target_set_nmi(0);
			target_set_nmi(1);

			delay_ms(250);
			target_set_ffck_level(1);
		}
		else
		{
			/* code */
			target_set_ffck_level(0);
			target_set_ffck_level(1);
		}
		

	}
	
	printf("End of interrupt test.\n");
	return 0;
}


enum new_reg
{
	NREG_SCLK = 0,
	NREG_MOSI,
	NREG_SSEL,
	NREG_MISO,
	NREG_SD_CD,
	NREG_TGT_WAITREQ,
	NREG_TGT_RDY,
	NREG_USER_LED,
};

const char *get_nreg_name(uint32_t offset)
{
	switch(offset)
	{
		case NREG_SCLK: return "SPI SCLK";
		case NREG_MOSI: return "SPI MOSI";
		case NREG_SSEL: return "SPI SSEL#";
		case NREG_MISO: return "SPI MISO";
		case NREG_SD_CD: return "SPI CD#";
		case NREG_TGT_WAITREQ: return "WAITREQ#";
		case NREG_TGT_RDY: return "65XX.RDY";
		case NREG_USER_LED: return "USER LED";
		default:
			return "<UNKNOWN>";
	}	
}

void write_newreg(uint32_t offset, uint8_t data)
{
	PIO_EXT[offset] = (data) ? 0x80 : 0x00;
}

uint8_t read_newreg(uint32_t offset)
{
	return (PIO_EXT[offset] & 0x80) ? 0x01 : 0x00;
}

/*----------------------------------------------------------------------------*/

#if 0
void target_set_power(uint8_t level)
{
	uint8_t waitreq = target_read_misc(7);
	uint8_t value = 0;
	
	if(waitreq)
		value |= 1;
	if(level)
		value |= 2;

	*PIO_REG_WAITREQ = value;
}

void target_set_waitreq(uint8_t level)
{
	uint8_t user_led = target_read_misc(4);
	uint8_t value = 0;
	
	if(level)
		value |= 1;
	if(user_led)
		value |= 2;

	*PIO_REG_WAITREQ = value;
}

#endif


class pio_interface
{
public:
	uint8_t dbrxl;
	uint8_t dbrxh;
	uint8_t ctol;
	uint8_t ctoh;
	uint8_t misc;
	uint8_t waitreq;
	
	pio_interface()
	{
		dbrxl = 0;
		dbrxh = 0;
		ctol = 0;
		ctoh = 0;
		misc = 0;
	}



	void flush(void)
	{
		*PIO_REG_CTL = ctol;
		*PIO_REG_CTH = ctoh;
		*PIO_REG_DBL = dbrxl;
		*PIO_REG_DBH = dbrxh;	
	}
};


int cmd_new(int argc, char *argv[])
{
	printf("Testing readback:\n");

	uint8_t value = 0x00;
	PIO_REG_DBL[0] = value;
	PIO_REG_DBH[0] = value;

	for(int i = 0; i < 8; i++)
	{
		uint8_t rold, rnew, rdelta;
		write_newreg(i, 1);
		rold = read_newreg(i);
		printf("Bit %d set=%d, ", i, rold);
		write_newreg(i, 0);
		rnew = read_newreg(i);
		rdelta = rold ^ rnew;
		printf("clr=%d, modifiable=%d [%s]\n", rnew, rdelta, get_nreg_name(i));
	}
	printf("Test complete.\n");
	write_newreg(NREG_USER_LED, 1);
	return 0;
}



int cmd_ex(int argc, char *argv[])
{
	void (*func)(void) = NULL;
	int number;

	if(argc < 2)
	{
		printf("Usage: %s <number>\n", argv[0]);
		return 0;
	}

	/* Get exception number from user */
	number = atoi(argv[1]);

	/* Point to vector table */
	uint32_t *rom_vectors = (uint32_t*)(0x00000000);

	/* Read vector from table */
	func = (void(*)(void))rom_vectors[number];
#if 0
	switch(number)
	{
		case 0:
		case 1:
		case 2:
			asm("move.w #0xdead, -(sp)\n"); /* dc.w type|fc */
			asm("move.l #0xaaaa5555, -(sp)\n"); /* dc.l access */
			asm("move.w #0xff55, -(sp)\n"); /* dc.w insn */
			asm("move.w #0xdead, -(sp)\n"); /* SR */
			break;
		case 3 ... 255:
			asm("move.w #0xdead, -(sp)\n"); /* SR */
			break;
		default:
			break;
	}
#endif
	// Note stack is not prepared here
	// function call will push pc (jsr)

	// group 1,2 stack sr,pch,pcl
	// group 0 stack
	// dc.w type,fc
	// dc.l access address
	// dc.w instruction
	// dc.w status
	// dc.l pc
	
	func();

	return 0;
}

int cmd_status(int argc, char *argv[])
{
	volatile uint8_t *status = (volatile uint8_t *)0xFF8081;
	uint8_t buf[0x100];

	for(int i = 0; i < 0x100; i++)
		buf[i] = status[0];

	printf("Status dump (%08X):\n", status);
	for(int i = 0; i < 0x100; i++)
	{
		if((i & 0x0F) == 0x00)
			printf("%04X: ", i);
		printf("%02X ", buf[i]);
		if((i & 0x0F) == 0x0F)
			printf("\n");
	}
}


/*
	0 - adl
	1 - adm
	2 - adh
	3 - data
	4 - dbrx		write 574
	5 - misc
	           write			read
	    D7 : 					0
	    D6 : 					0
	    D5 : 					card_detect_n
	    D4 : 					hirq2_n
	    D3 : 	hreset#			hreset#
	    D2 : 	hsm				hsm
	    D1 : 	cd_rx			cd_rx
	    D0 :	hirq2_rx 		hirq2_rx
	6 - lfsr
	7 - id (55)
*/


/* Register numbers */
#define HUREG_ADL		0x00
#define HUREG_ADM		0x01
#define HUREG_ADH		0x02
#define HUREG_DATA		0x03
#define HUREG_DBRX		0x04
#define HUREG_MISC		0x05
#define HUREG_DEBUG		0x06
#define HUREG_ID		0x07

/* Register bits */
#define B_HIRQ2_RX		0
#define B_CD_RX			1
#define B_HSM			2
#define B_RESET			3
#define B_HIRQ2			4
#define B_CD			5

class huread
{
public:
	uint8_t reg[8];

	void write(uint8_t offset, uint8_t data)
	{
		reg[offset] = data;
		PIO_EXT[offset] = data;
	}

	uint8_t read(uint8_t offset)
	{
		return PIO_EXT[offset];
	}

	void change_bit(uint8_t offset, uint8_t bit, bool level)
	{
		uint8_t temp = reg[offset];
		temp &= ~(1 << bit);
		if(level)
		{
			temp |= (1 << bit);
		}

		write(offset, temp);
	}

	bool get_bit(uint8_t offset, uint8_t bit)
	{
		uint8_t temp = PIO_EXT[offset];
		
		if(temp & (1 << bit))
		{
			return true;	
		}

		return false;
	}

	bool get_card_detect(void)
	{
		return get_bit(HUREG_MISC, B_CD);
	}

	bool get_irq2(void)
	{
		return get_bit(HUREG_MISC, B_HIRQ2);
	}

	void set_hsm(bool level)
	{
		change_bit(HUREG_MISC, B_HSM, level);
	}

	void set_reset(bool level)
	{
		change_bit(HUREG_MISC, B_RESET, level);
	}

	void set_card_detect_rx(bool level)
	{
		change_bit(HUREG_MISC, B_CD_RX, level);
	}

	void set_irq2_rx(bool level)
	{
		change_bit(HUREG_MISC, B_HIRQ2_RX, level);
	}

	void set_address(uint32_t address)
	{
		write(HUREG_ADL, (address >>  0) & 0xFF);
		write(HUREG_ADM, (address >>  8) & 0xFF);
		write(HUREG_ADH, (address >> 16) & 0x1F);
	}

	void set_dbrx(uint8_t level)
	{
		write(HUREG_DBRX, level);
	}

	uint8_t card_write(uint32_t address, uint8_t data)
	{
		set_address(address);
		write(HUREG_DATA, data);
	}

	uint8_t card_read(uint32_t address)
	{
		set_address(address);
		return read(HUREG_DATA);
	}

	void initialize(void)
	{
		set_address(0x000000);
		set_dbrx(0xFF);
		set_card_detect_rx(true);
		set_irq2_rx(true);
		set_hsm(false);
		set_reset(false);
	}

	void read_page(uint8_t *page)
	{
		for(uint16_t i = 0; i < 0x100; i++)
		{
			PIO_EXT[HUREG_ADL] = i;
			page[i] = PIO_EXT[HUREG_DATA];
		}
	}


	void read_page_auto(uint8_t *page)
	{
		PIO_EXT[HUREG_ADL] = 0xFF;
		for(uint16_t i = 0; i < 0x100; i++)
		{
			page[i] = PIO_EXT[7];
		}
	}

};

extern "C" void huread_page_auto(uint8_t *ptr);
extern "C" uint8_t __kernel_page_buffer[256];

uint8_t rom_bank[0x2000];

int cmd_hu(int argc, char *argv[])
{
	uint8_t temp;
	huread dev;

	printf("Start of test.\n");



	printf("Start of test.\n");

	dev.initialize();


	dev.set_card_detect_rx(false);
	printf("Drive CD# lo, readback: %d\n", dev.get_card_detect());
	dev.set_card_detect_rx(true);
	printf("Drive CD# hi, readback: %d\n", dev.get_card_detect());

	dev.set_irq2_rx(false);
	printf("Drive IRQ2# lo, readback: %d\n", dev.get_irq2());
	dev.set_irq2_rx(true);
	printf("Drive IRQ2# hi, readback: %d\n", dev.get_irq2());

	temp = dev.get_card_detect();
	printf("CD#   = %d\n", temp);
	temp = dev.get_irq2();
	printf("IRQ2# = %d\n", temp);

	dev.set_dbrx(0xA5);
	uint32_t base = 0x1f00;
	uint8_t page[0x100];

	dev.set_hsm(false);
	dev.set_reset(true);
	delay_ms(100);
	dev.set_reset(false);
	dev.set_hsm(true);

	comms_mutex.lock();

#if 1
	pc_puts("STATUS: Reading HuCard image.\n");
	int fd = pc_fopen("card.bin", "wb");
	for(int bank_num = 0; bank_num < 0x80; bank_num++)
	{

		// read bank
		for(int page_num = 0; page_num < 0x20; page_num++)
		{
			dev.set_address(bank_num * 8192 + page_num * 0x100);
			dev.read_page(rom_bank + (page_num * 0x100));
		}

		pc_fwrite(fd, rom_bank, 0x2000);
	}
	pc_fclose(fd);
	pc_exit();

	comms_mutex.unlock();
#else
/*
	printf("send exit..\n");
	pc_puts("open file\n");
	int k = pc_fopen("debug.txt", "wb");
	pc_puts("close file\n");
	pc_fwrite(k, (uint8_t *)0, 0x8000);
	pc_fclose(k);

	pc_exit();
	printf("post exit...\n");

*/

	dev.set_address(0x1f00);	
	dev.read_page(page);
	//huread_page_auto(page);

	// Print buffer
	for(int i = 0; i < 0x100; i++)
	{
		uint32_t offset = base + i;
		if((i & 0x0F) == 0x00)
			printf("%08X : ", offset);
		printf("%02X ", page[i]);
		if((i & 0x0F) == 0x0F)
			printf("\n");
	}
#endif
	printf("End of test.\n");
	return 0;
}

int cmd_uptime(int argc, char *argv[])
{
	printf("Uptime: %08X ticks.\n", __systick_count);
	return 0;
}



cli_cmd_t terminal_cmds[] = 
{
	{"?",       cmd_list},
	{"info", 	cmd_info},
	{"reset", 	cmd_reset},
	{"reboot", 	cmd_reboot},
	{"mem", 	cmd_mem},
	{"memdump", cmd_memdump},
	{"flash",   cmd_flash},
	{"led",     cmd_led},
	{"post",    cmd_post},
	{"debug", 	cmd_debug},
	{"cls",     cmd_clrscr},
	{"clear",   cmd_clrscr},
	{"spi",     cmd_spi},
//	{"pio",     cmd_pio},
	{"piodbus", cmd_piodbus},
	{"pioram",  cmd_pioram},
	{"alloc",   cmd_alloc},
	{"isr",     cmd_isr},
	{"iotest",  cmd_iotest},
	{"timer",   cmd_timer},
	{"probein", cmd_probein},
	{"probeout",cmd_probeout},
	{"boot",    cmd_boot},
	{"zdiag",   cmd_zdiag},
	{"step",    cmd_step},
	{"edge", 	cmd_edge},
	{"nmi",		cmd_nmi},
	{"int",     cmd_int},
	{"new",     cmd_new},
	{"ex",      cmd_ex},
	{"status",  cmd_status},
	{"hu",      cmd_hu},
	{"uptime",  cmd_uptime},
	{NULL, 		NULL}
};


#endif