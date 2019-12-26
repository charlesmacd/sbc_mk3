
#ifndef _SYSTEM_CONTROLLER_H_
#define _SYSTEM_CONTROLLER_H_
 
#ifndef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include "../hw_defs.hpp"
#include "../sys_types.hpp"

/* Board-specific constants */
constexpr uint32_t kSystemPeripheralClock   =  8'000'000;
constexpr uint32_t kSystemCPUClock          = 12'000'000;
constexpr uint32_t kSystemControllerRegBase = 0xFFFF8000;

#define PERIPHERAL_UART         0
#define PERIPHERAL_PCF          1

/* STATUS flags */
#define SYSCON_STATUS_BREAK     0x01
#define SYSCON_STATUS_SPI_MISO  0x02
#define SYSCON_STATUS_DMA_GO    0x04
#define SYSCON_STATUS_UART_INT  0x08
#define SYSCON_STATUS_PCF_INT   0x10
#define SYSCON_STATUS_USB_PWE   0x20
#define SYSCON_STATUS_USB_TXE   0x40
#define SYSCON_STATUS_USB_RXF   0x80

/* ID value */
#define SYSCON_ID               0x81

/* GPIO ports */
#define SYSCON_PORT_OD          0x00
#define SYSCON_PORT_SS          0x01

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

struct system_controller_register_tag
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

typedef system_controller_register_tag system_controller_register_t;

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/


class SystemController
{
public:
    system_controller_register_t *sysc;
    uint32_t peripheral_clock;
    uint32_t cpu_clock;

public:
    void initialize(system_controller_register_t *external_reg = NULL)
    {
        sysc = external_reg;
        if(sysc == NULL)
        {
            sysc = (system_controller_register_t *)kSystemControllerRegBase;
        }
        
        peripheral_clock = kSystemPeripheralClock;
        cpu_clock = kSystemCPUClock;
    }
    
    /* Get the peripheral clock in Hz */
    uint32_t get_peripheral_clock(void)
    {
        return peripheral_clock;
    }

    /* Get the CPU clock in Hz */
    uint32_t get_cpu_clock(void)
    {
        return cpu_clock;
    }

    /* Read the ID register */
    uint8_t get_id(void)
    {
        return sysc->reg.r.ID;
    }

    /* Read the status register */
    uint8_t get_status(void)
    {
        return sysc->reg.r.STATUS;
    }

    uint8_t exchange_spi(uint8_t data)
    {

        sysc->reg.w.OUT1.SPI_SSEL_N = 0;

        /* TODO */

        sysc->reg.w.OUT1.SPI_SSEL_N = 1;

        return sysc->reg.r.SPI_DATA;
    }

    /* Write to a GPIO port */
    void write_gpio(uint8_t port, uint8_t bit, uint8_t state)
    {
        switch(port)
        {
            case SYSCON_PORT_OD: /* Open-drain outputs (2) */
                sysc->reg.w.OUT1.OUT_OD[bit & 1] = (state) ? 0x01 : 0x00;
                break;
            case SYSCON_PORT_SS: /* Sink-source outputs (4) */
                sysc->reg.w.OUT1.OUT_SS[bit & 3] = (state) ? 0x01 : 0x00;
                break;
        }
    }

    /* Control the GATE inputs of the 82C55 PIT */
    void set_timer_gate(uint8_t channel, bool state)
    {
        switch(channel)
        {
            case 0:
                sysc->reg.w.OUT0.INTERVAL_1US_ENA = (state) ? 0x01 : 0x00;
                break;
            case 1:
                sysc->reg.w.OUT0.INTERVAL_1MS_ENA = (state) ? 0x01 : 0x00;
                break;
            case 2:
                /* SYSTICK is always enabled */
                break;
        }
    }

    /* Control the user LED */
    void set_user_led(bool enabled)
    {
        sysc->reg.w.OUT0.USER_LED_K = (enabled) ? 1 : 0;
    }

    /* Set an 8-bit POST code */
    void set_post(uint8_t value)
    {
        sysc->reg.w.POST = value;
    }

    /* Control the POST digit decimal point LEDs */
    void set_post_dp(uint8_t mask)
    {
        for(int i = 0; i < 4; i++)
        {
            sysc->reg.w.OUT0.DP[i] = (mask & (1 << i)) ? 0 : 1;
        }
    }

    /* Assert a reset output line */
    void assert_peripheral_reset(int which, bool state)
    {
        switch(which)
        {
            case PERIPHERAL_UART:
                sysc->reg.w.OUT0.UART_RESET = (state) ? 1 : 0;
                break;
            case PERIPHERAL_PCF:
                sysc->reg.w.OUT1.PCF_RESET_N = (state) ? 1 : 0;
                break;
        }
    }
};


#ifndef __cplusplus
}; /* extern */
#endif

#endif /* _SYSTEM_CONTROLLER_HPP_ */