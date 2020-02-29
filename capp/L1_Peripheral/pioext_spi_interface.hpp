
#ifndef _PIOEXT_SPI_INTERFACE_HPP_
#define _PIOEXT_SPI_INTERFACE_HPP_

#include "host_spi_interface.hpp"





class pioext_spi_interface : public HostSPIInterface
{
private:
	static constexpr uint8_t word_length = 8;
	static constexpr uint8_t word_mask = (word_length - 1);

	static constexpr uint8_t kSCLK = 0x00;
	static constexpr uint8_t kMOSI = 0x01;
	static constexpr uint8_t kSSEL = 0x02;
	static constexpr uint8_t kRST  = 0x03;
	static constexpr uint8_t kMISO = 0x07;

	uint8_t clock_polarity;
	uint8_t clock_phase;
	SBCPIO_EXT &intf;

	void clock(void)
	{
		intf.write(kSCLK, clock_polarity ^ 1);
		intf.write(kSCLK, clock_polarity ^ 0);
	}

public:

	pioext_spi_interface(SBCPIO_EXT &intf_) : intf(intf_)
	{
		;
	}

	void set_mode(uint8_t mode)
	{
		clock_polarity = (mode >> 0) & 1;
		clock_phase = (mode >> 1) & 1;
		intf.write(kSCLK, clock_polarity);
		intf.write(kMOSI, 0);
		intf.write(kSSEL, 1);
	}

	void initialize(void)
	{
		intf.initialize();
		/* */
	}

	uint8_t exchange(uint8_t value)
	{
		uint8_t temp = 0;
		for(int i = 0; i < word_length; i++)
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

		for(int i = 0; i < word_length; i++)
		{
			intf.write(kSCLK, clock_polarity ^ 1);
			temp |= intf.read_bit(kMISO, 0) << (i ^ 7);
			intf.write(kSCLK, clock_polarity ^ 0);
		}		
		return temp;		
	}

	void write(uint8_t value)
	{
		for(int i = 0; i < word_length; i++)
		{
			intf.write(kMOSI, (value >> (i ^ 7)));
			clock();
		}		
	}

	void select(bool is_asserted)
	{
		intf.write(kSSEL, is_asserted ? 0 : 1);
	}

	void assert_reset(bool is_asserted)
	{
		intf.write(kRST, is_asserted ? 0 : 1);
	}
};

#endif /* _PIOEXT_SPI_INTERFACE_HPP_ */
