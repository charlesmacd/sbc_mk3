
#ifndef _HOST_SPI_INTERFACE_HPP_
#define _HOST_SPI_INTERFACE_HPP_

class HostSPIInterface
{
public:
	virtual void set_mode(uint8_t mode) = 0;
	virtual void initialize(void) = 0;
	virtual uint8_t exchange(uint8_t value) = 0;
	virtual uint8_t read(void) = 0;
	virtual void write(uint8_t value) = 0;
	virtual void select(bool is_asserted) = 0;
	virtual void assert_reset(bool is_asserted) = 0;
};

#endif /* _HOST_SPI_INTERFACE_HPP_ */