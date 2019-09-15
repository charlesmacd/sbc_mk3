
#ifndef _TIMER_H_
#define _TIMER_H_


#define PIT_CNT_EVENT		0x00
#define PIT_CNT_HW_1SHOT	0x01
#define PIT_CNT_REPEAT		0x02
#define PIT_CNT_SQW			0x03
#define PIT_CNT_SW_TRG		0x04
#define PIT_CNT_HW_TRG		0x05

#define PIT_CNT_BINARY		0x00
#define PIT_CNT_BCD			0x01

#define PIT_LOAD_LOW		0x00
#define PIT_LOAD_HIGH		0x10
#define PIT_LOAD_WORD		0x30

#define PIT_CTRL(counter, mode, type)	(counter << 3) | (mode << 1) | (type)

/* Function prototypes */
void pit_write_counter(uint8_t channel, uint16_t count);
uint16_t pit_read_counter(uint8_t channel);
void set_1ms_timer(uint16_t count);
void set_1us_timer(uint16_t count);
void timer_1ms_enable(bool enabled);
void timer_1us_enable(bool enabled);
void timer_init(void);
void delay_ms(uint16_t duration);

#endif /* _TIMER_H_ */


