

#include <stdlib.h>
#include "system_controller.hpp"
#include "../newlib/printf.hpp"

void debug_validate_addresses(void)
{
	printf("size=%d\n", sizeof(system_controller_register_t));

	system_controller_register_t *q = (system_controller_register_t *)0xFFFF8000;

	printf("L:%08X\n", (uint32_t)&q->reg.w.POST);
	
    printf("OUT0 latch:\n");
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT0.INTERVAL_1US_ENA);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT0.INTERVAL_1MS_ENA);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT0.UART_RESET);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT0.USER_LED_K);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT0.DP[0]);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT0.DP[1]);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT0.DP[2]);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT0.DP[3]);

    printf("OUT1 latch:\n");
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT1.PCF_RESET_N);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT1.OUT_OD[0]);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT1.OUT_OD[1]);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT1.SPI_SSEL_N);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT1.OUT_SS[0]);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT1.OUT_SS[1]);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT1.OUT_SS[2]);
	printf("L:%08X\n", (uint32_t)&q->reg.w.OUT1.OUT_SS[3]);
	
	
	printf("L:%08X\n", (uint32_t)&q->reg.r.SPI_CLOCKS);
	printf("L:%08X\n", (uint32_t)&q->reg.r.SPI_DATA);
	printf("L:%08X\n", (uint32_t)&q->reg.r.STATUS);
}



/* End */