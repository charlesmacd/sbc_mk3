
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "sbc.h"
#include "cli.h"
#include "cli_cmd.h"
#include "uart.h"



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
volatile uint8_t  *PIO_EXT =  (volatile uint8_t *)(PIO_EXT_BASE);
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

void target_set_clock_mode(uint8_t mode)
{
	gcth &= ~F_CLKSEL_MASK;

	if(mode & 1)
		gcth |= F_CLKSEL0;
	if(mode & 2)
		gcth |= F_CLKSEL1;

	PIO_REG_CTH[0] = gcth;
}

void target_set_data(uint16_t data)
{
	PIO_REG_DBL[0] = (data >> 0) & 0xFF;
	PIO_REG_DBH[0] = (data >> 8) & 0xFF;
}

void target_assert_reset(bool state)
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

void target_set_ipl(int level)
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



/* Display software information */
static int cmd_info(int argc, char *argv[])
{
	printf("Hardware:   SBC-V4\n");
	printf("Build date: %s\n", __DATE__);
	printf("Build time: %s\n", __TIME__);

#if 0
#endif
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

/* Display memory information */
static int cmd_mem(int argc, char *argv[])
{
	extern char *__stack;
	extern char *_text_start;
	extern char *_data_start;
	extern char *_bss_start;
	extern uint32_t _stext;
	extern uint32_t _sdata;
	extern uint32_t _sbss;

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
	printf("* _end    : %08X\n", (uint32_t)&_end);
	printf("* __stack : %08X\n", (uint32_t)&__stack);

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
		uint8_t value = strtoul(argv[1], NULL, 16);
		set_post(value);
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



extern uint32_t pic_flash_library_base;

void (*pic_flash_write)(uint32_t address, uint32_t num_pages, uint8_t *data) =  (void *)(&pic_flash_library_base + 0x00);
void (*pic_flash_read)(uint32_t address, uint32_t num_pages, uint8_t *data)  =  (void *)(&pic_flash_library_base + 0x01);

static int cmd_debug(int argc, char *argv[])
{
	uint8_t *buffer = (uint8_t *)0x104000;

	for(int i = 0; i < 256; i++)
			buffer[i] = 0x80 ^ i;

	pic_flash_write(0x200000, 0x80, buffer);

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
	{"pio",     cmd_pio},
	{"piodbus", cmd_piodbus},
	{"pioram",  cmd_pioram},
	{"alloc",   cmd_alloc},
	{"isr",     cmd_isr},
	{NULL, 		NULL}
};


