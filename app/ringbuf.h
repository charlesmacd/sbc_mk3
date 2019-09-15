
#ifndef _RINGBUF_H_
#define _RINGBUF_H_

#include <stdint.h>

#define RINGBUF_MAX_SIZE		128

typedef struct
{
	uint8_t capacity;
	uint8_t mask;
	uint8_t write;
	uint8_t read;
	uint8_t underflow;
	uint8_t overflow;
	uint8_t buffer[RINGBUF_MAX_SIZE];
} ringbuf_t;



void ringbuf_clear_overflow(ringbuf_t *p);
void ringbuf_clear_underflow(ringbuf_t *p);
void ringbuf_init(ringbuf_t *p, uint32_t capcity);
void ringbuf_insert(ringbuf_t *p, uint8_t data);
uint8_t ringbuf_remove(ringbuf_t *p);
int ringbuf_empty(ringbuf_t *p);
uint8_t ringbuf_size(ringbuf_t *p);
int ringbuf_full(ringbuf_t *p);


#endif /* _RINGBUF_H_ */
