

#include <stdio.h>
#include "../L1_Peripheral/system_controller.hpp"
#include "../L3_Application/ring_buf.hpp"
#include "../sbc.hpp"
#include "uart.hpp"

uint32_t __popcount32(uint32_t value)
{
	return __builtin_popcount(value);
}

SystemController system_controller;
InterruptController interrupt_controller;
Uart uart;


int main (int argc, char *argv[])
{
	uint8_t sc_reg_reset;
	system_controller_register_t syscon_reg;
	syscon_reg.REG_UART_RESET = &sc_reg_reset;

	interrupt_controller_register_t intcon_reg;
	system_controller.initialize(&syscon_reg);
	interrupt_controller.initialize(&intcon_reg);  


	return 0;
}