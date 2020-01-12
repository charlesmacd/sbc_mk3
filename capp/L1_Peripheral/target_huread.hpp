


#ifndef _TARGET_HUREAD_HPP_
#define _TARGET_HUREAD_HPP_

#include "sbc_pio.hpp"



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



class Target_HuRead
{
public:
    SBCPIO &sbc_pio;
    sbcpio_ext_register_t *intf;
    uint8_t reg[8];

    Target_HuRead(SBCPIO &ref) : sbc_pio(ref)
    {
        intf = sbc_pio.ext_intf;
    }

    void assign(SBCPIO &ref)
    {
        intf = sbc_pio.ext_intf;
    }   

	void write(uint8_t offset, uint8_t data)
	{
		reg[offset] = data;
		intf->reg.w.REG[offset] = data;
	}

	uint8_t read(uint8_t offset)
	{
		return intf->reg.r.REG[offset];
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
		uint8_t temp = read(offset);
		
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
	    return 0;
	}

    uint8_t strobe_read(void)
    {
        return read(HUREG_DATA);
    }

    void strobe_write(uint8_t data)
    {
        write(HUREG_DATA, data);
    }

	uint8_t card_read(uint32_t address)
	{
		set_address(address);
		return read(HUREG_DATA);
	}

    uint8_t read_dbus(void)
    {
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
			intf->reg.w.REG[HUREG_ADL] = i;
			page[i] = intf->reg.r.REG[HUREG_DATA];
		}
	}


	void read_page_auto(uint8_t *page)
	{
		intf->reg.w.REG[HUREG_ADL] = 0xFF;
		for(uint16_t i = 0; i < 0x100; i++)
		{
			page[i] = intf->reg.r.REG[HUREG_ID]; // now auto data
		}
	}

};


#endif /* _TARGET_HUREAD_HPP_ */