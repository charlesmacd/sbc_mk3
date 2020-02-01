


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

	void assert_hsm(bool state)
	{
		change_bit(HUREG_MISC, B_HSM, state ? 1 : 0);
	}

	void assert_reset(bool state)
	{
		change_bit(HUREG_MISC, B_RESET, state ? 0 : 1);
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

	void set_data_pullups(uint8_t level)
	{
		write(HUREG_DBRX, level);
	}

	uint8_t card_write(uint32_t address, uint8_t data)
	{
		set_address(address);
		write(HUREG_DATA, data);
	    return 0;
	}

	void card_write_logical(uint8_t bank, uint16_t address, uint8_t data)
	{
		set_address(bank * 0x2000 + (address & 0x1FFF));
		strobe_write(data);
		printf("A:%02X %02X %02X = %02X\n", read(HUREG_ADH), read(HUREG_ADM), read(HUREG_ADL), data);
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

	uint16_t *get_auto_port(void)
	{
		return (uint16_t *)&intf->reg.r.REG[HUREG_ID];
	}

    uint8_t read_dbus(void)
    {
        return read(HUREG_DATA);
    }

	uint8_t strobe_read_auto(void)
	{
		return read(HUREG_ID);
	}

	void initialize(void)
	{
		set_address(0x000000);
		set_data_pullups(0xFF);
		set_card_detect_rx(true);
		set_irq2_rx(true);
		assert_hsm(false);
		assert_reset(false);
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