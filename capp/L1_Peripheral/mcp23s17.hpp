
#ifndef _MCP23S17_HPP_
#define _MCP23S17_HPP_

#include <stdint.h>
#include "host_spi_interface.hpp"

enum MCP23S17_Port : uint8_t
{
	PORTA,
	PORTB,
};

enum MCP23S17_Regs : uint8_t
{
    IODIRA  =   0x00,
    IODIRB,
    IPOLA,
    IPOLB,
    GPINTENA,
    GPINTENB,
    DEFVALA,
    DEFVALB,
    INTCONA,
    INTCONB,
    IOCONA,
    IOCONB,
    GPPUA,
    GPPUB,
    INTFA,
    INTFB,
    INTCAPA,
    INTCAPB,
    GPIOA,
    GPIOB,
    OLATA,
    OLATB,
};


class mcp23s17
{

public:
	HostSPIInterface &intf;
	uint8_t register_copy[8][0x20];

	mcp23s17(HostSPIInterface &intf_) : intf(intf_)
	{
		;
	}

	void initialize(void)
	{
		this->intf.initialize();		
	}

	void reset(void)
	{
		
		/* Pulse hardware reset pin */
		intf.assert_reset(true);		
		for(int i = 0; i < 0x10; i++)
		{
			; /* TODO NEED DELAY */
		}

		/* Reset register copy state */
		memset(register_copy, 0x00, sizeof(register_copy));
		for(int i = 0; i < 8; i++)
		{
			register_copy[i][IODIRA] = 0xFF;
			register_copy[i][IODIRB] = 0xFF;
		};

		intf.assert_reset(false);
		for(int i = 0; i < 0x10; i++)
		{
			; /* TODO NEED DELAY */
		}

		/* Broadcast IOCON write to all devices to enable HAEN */
		for(int i = 0; i < 8; i++)
		{
			write_reg(i, MCP23S17_Regs::IOCONA, 0x08);
		}
	}

	void write_reg(uint8_t device_num, uint8_t reg_num, uint8_t reg_value)
	{
		device_num &= 0x07;
		reg_num &= 0x1F;
		register_copy[device_num][reg_num] = reg_value;

		constexpr uint8_t format = 0x40;
		intf.select(true);
		intf.write(format | ((device_num & 7) << 1));
		intf.write(reg_num);
		intf.write(reg_value);
		intf.select(false);	
	}

	uint8_t read_reg(uint8_t device_num, uint8_t reg_num)
	{
		uint8_t temp;
		constexpr uint8_t format = 0x41;
		intf.select(true);
		intf.write(format | ((device_num & 7) << 1));
		intf.write(reg_num & 0x1F);
		temp = intf.read();
		intf.select(false);			
		return temp;
	}

	uint8_t get_last_reg(uint8_t device_num, uint8_t reg_num)
	{
		return register_copy[device_num & 0x07][reg_num & 0x1F];
	}

	void write_reg_bit(uint8_t device_num, uint8_t reg_num, uint8_t bit, uint8_t value)
	{
		uint8_t temp = get_last_reg(device_num, reg_num);
		value = (value & 1) << (bit & 7);
		temp &= ~value;
		if(value)
		{
			temp |= value;
		}
		write_reg(device_num, reg_num, value);
	}
};


#endif /* _MCP23S17_HPP_ */
