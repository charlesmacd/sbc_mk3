

#ifndef _SBC_PIO_H_
#define _SBC_PIO_H_

#ifndef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../hw_defs.hpp"
#include "../sys_types.hpp"
#include "parallel_memory.hpp"

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

enum PIOSignalDir : uint8_t
{
	kOUTPUT,
	kINPUT,
	kBIDIR,
};

enum PIOSignalAttr : uint8_t
{
	kCLOCK,
	kWSG_IN,
	kWSG_OUT,
	kNONE,
};

enum PIORegName : uint8_t
{
	kADL,
	kADM,
	kADH,
	kDBL,
	kDBH,
	kCTL,
	kCTH,
	kMISC
};

struct PIOSignalInfo_tag
{
	PIOSignalName name;
	PIOSignalDir dir;
	PIOSignalAttr attr;
	PIORegName reg_num;
	uint8_t reg_bit;
};

typedef PIOSignalInfo_tag PIOSignalInfo_t;





/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#define USE_ASM		1

/*----------------------------------------------------------------------------*/
/* SBCPIO: Clocking register */
/*----------------------------------------------------------------------------*/

struct sbcpio_ffck_register_tag
{
	union
	{
		struct
		{
			__OM uint16_t CLK;
		} w;
	} reg;
};

typedef sbcpio_ffck_register_tag sbcpio_ffck_register_t;

/*----------------------------------------------------------------------------*/
/* SBCPIO: Expansion register */
/*----------------------------------------------------------------------------*/

struct sbcpio_ext_register_tag
{
	union
	{
		struct
		{
			__OM uint16_t REG[8];
		} w;
		struct
		{
			__IM uint16_t REG[8];
		} r;
	} reg;
};

typedef sbcpio_ext_register_tag sbcpio_ext_register_t;

/*----------------------------------------------------------------------------*/
/* SBCPIO: PIO register */
/*----------------------------------------------------------------------------*/

struct sbcpio_register_tag
{
	union
	{
		struct
		{
			__OM uint16_t WAITREQ_LD;	
			__OM uint16_t CLK_CLR;	
			__OM uint16_t CLK_PRE;	
			__OM uint16_t DBL_RX;	
			__OM uint16_t DBH_RX;	
			__OM uint16_t CTL;	
			__OM uint16_t CTH;	
			__OM uint16_t MISC;
		} w;

		struct
		{
			__IM uint16_t ADL;	
			__IM uint16_t ADM;	
			__IM uint16_t ADH;	
			__IM uint16_t DBL;	
			__IM uint16_t DBH;	
			__IM uint16_t CTL;	
			__IM uint16_t CTH;	
			__IM uint16_t MISC;	
		} r;	
	} reg;
} __attribute__((packed, aligned(2)));

typedef sbcpio_register_tag sbcpio_register_t;

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

class SBCPIO_EXT
{
private:
	static constexpr uint8_t kRegCount = 8;
	static constexpr uint8_t kRegMask = (kRegCount - 1);
	static constexpr uint32_t kExtRegBase  = 0x904000;		

	sbcpio_ext_register_t *intf;
	uint8_t reg[kRegCount];
public:

	void initialize(sbcpio_ext_register_t *external_reg = NULL)
	{
		intf = external_reg;
		if(intf == NULL)
		{
			intf = (sbcpio_ext_register_t *)kExtRegBase;
		}

		/* Clear internal history of register writes */
		memset(reg, 0x00, kRegCount);
	}

	void write(uint8_t offset, uint8_t data)
	{
		reg[offset & kRegMask] = data;
		intf->reg.w.REG[offset] = data;
	}

	uint8_t read_last_written(uint8_t offset)
	{
		return reg[offset & kRegMask];
	}

	uint8_t read(uint8_t offset)
	{
		return intf->reg.r.REG[offset & kRegMask];
	}

	/* NOTE: Reads back I/O register before writing new value */
	void modify_bit(uint8_t offset, uint8_t position, uint8_t value)
	{
		uint8_t temp = read(offset);
		if(value)
		{
			temp |= (1 << position);
		} 
		else
		{
			temp &= ~(1 << position);
		}
		write(offset, temp);
	}

	/* NOTE: Reads back last-written value to compute modified value */
	void write_bit(uint8_t offset, uint8_t position, uint8_t value)
	{
		uint8_t temp = read_last_written(offset);
		if(value)
		{
			temp |= (1 << position);
		} 
		else
		{
			temp &= ~(1 << position);
		}
		write(offset, temp);
	}

	uint8_t read_bit(uint8_t offset, uint8_t position)
	{
		return (read(offset) >> position) & 1;
	}
};


/*----------------------------------------------------------------------------*/

class SBCPIO
{
public:
    sbcpio_register_t *intf;
	sbcpio_ffck_register_t *ffck_intf;
	sbcpio_ext_register_t *ext_intf;
	SBCPIO_EXT ext;
	ParallelMemory ram;

	void initialize(sbcpio_register_t *external_reg = NULL)
	{
		constexpr uint32_t kExtRegBase  = 0x904000;
		constexpr uint32_t kFFCKRegBase = 0x908000;
		constexpr uint32_t kRegBase     = 0x90C000;
		constexpr uint32_t kRAMBase     = 0x910000;
		constexpr uint32_t kRAMSize		= 0x008000; /* CY7C199 : 32Kx8 */

		intf = external_reg;
		if(intf == NULL)
		{
			ext_intf = (sbcpio_ext_register_t *)kExtRegBase;
			ffck_intf = (sbcpio_ffck_register_t *)kFFCKRegBase;
			intf = (sbcpio_register_t *)kRegBase;
			ram.initialize(kRAMBase, kRAMSize);
		}

		ext.initialize((sbcpio_ext_register_t *)kExtRegBase);
	}

	/* Pulse FFCK multiple times */
	void toggle_ffck(uint32_t count)
	{
		#if USE_ASM		
		__asm__ __volatile__ ("movep.l %0, 0(%1)\n" : : "r"(0), "a"(ffck_intf->reg.w.CLK) : );
		#else
		ffck_intf->reg.w.CLK = 0;
		ffck_intf->reg.w.CLK = 0;
		#endif
	}

	/* Set FFCK state */
	void set_ffck_level(int value)
	{
		if(value)
		{
			intf->reg.w.CLK_PRE = 1;
		}
		else
		{
			intf->reg.w.CLK_CLR = 1;
		}
	}

	/* Read address */
	uint32_t read_address(void)
	{
		uint32_t value;
		#if USE_ASM
		__asm__ __volatile__ ("movep.l 0(%1), %0\n" "andi.l #0xFFFFFE, %0\n" : : "r"(value), "a"(intf->reg.r.ADL) : );
		#else
		value |= (intf->reg.r.ADL & 0xFE) << 0;
		value |= (intf->reg.r.ADM & 0xFF) << 8;
		value |= (intf->reg.r.ADH & 0xFF) << 16;
		#endif
		return value;
	}

	/* Read data */
	uint16_t read_data(void)
	{
		uint16_t value;
		#if USE_ASM
		__asm__ __volatile__ ("movep.w 0(%1), %0\n" : : "r"(value), "a"(intf->reg.r.DBL) : );
		#else
		value |= (intf->reg.r.DBL & 0xFE) << 0;
		value |= (intf->reg.r.DBH & 0xFF) << 8;
		#endif
		return value;
	}

	/* Set data bus pull-up level */
	void set_data_pullup_level(uint16_t value)
	{
		#if USE_ASM
		__asm__ __volatile__ ("movep.w %0, 0(%1)\n" : : "r"(value), "a"(intf->reg.w.DBL_RX) : );
		#else
		intf->reg.w.DBL_RX = (value >> 0) & 0xFF;
		intf->reg.w.DBH_RX = (value >> 8) & 0xFF;
		#endif
	}

	void assert_signal(void)
	{
	
	}
};

#ifndef __cplusplus
}; /* extern */
#endif

#endif /* _SBC_PIO_HPP_ */