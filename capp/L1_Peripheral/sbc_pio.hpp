

#ifndef _SBC_PIO_H_
#define _SBC_PIO_H_

#ifndef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include "../hw_defs.hpp"
#include "../sys_types.hpp"

struct sbc_pio_register_tag
{
	union
	{
		struct
		{
			/* 0x00 */
			__OM uint16_t POST;
			STRUCT_PADDING(0, 0x40, 1);

			/* 0x40 */
			struct 
			{
				__OM uint16_t INTERVAL_1US_ENA;
				__OM uint16_t INTERVAL_1MS_ENA;
				__OM uint16_t UART_RESET;
				__OM uint16_t USER_LED_K;
				__OM uint16_t DP[4];
			} OUT0;
			STRUCT_PADDING(1, 0x40, 8);

			/* 0x80 */
			struct
			{
				__OM uint16_t PCF_RESET_N;
				__OM uint16_t OUT_OD[2];
				__OM uint16_t SPI_SSEL_N;
				__OM uint16_t OUT_SS[4];
			} OUT1;
			STRUCT_PADDING(2, 0x40, 8);

			/* 0xC0 */
			STRUCT_PADDING(3, 0x40, 0);
		} w;

		struct
		{
			/* 0x00 */
			__IM uint16_t ID;
			STRUCT_PADDING(0, 0x40, 1);

			/* 0x40 */
			__IM uint16_t STATUS;
			STRUCT_PADDING(1, 0x40, 1);

			/* 0x80 */
			__IM uint16_t SPI_CLOCKS;
			STRUCT_PADDING(2, 0x40, 1);

			/* 0xC0 */
			__IM uint16_t SPI_DATA;
			STRUCT_PADDING(3, 0x40, 1);
		} r;	
	} reg;
} __attribute__((packed, aligned(2)));

typedef sbc_pio_register_tag sbc_pio_register_t;


class sbc_pio
{
public:
    sbc_pio_register_t *intf;
};





#ifndef __cplusplus
}; /* extern */
#endif

#endif /* _SBC_PIO_HPP_ */