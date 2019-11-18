/*
	File:
		ringbuf.c
	Author:
		Charles MacDonald
	Notes:
		None
*/


#include <stdint.h>
#include "ringbuf.h"


/* Clear oveflow condition */
void ringbuf_clear_overflow(ringbuf_t *p)
{
	p->overflow = 0;
}



/* Clear underflow condition */
void ringbuf_clear_underflow(ringbuf_t *p)
{
	p->underflow = 0;
}



/* Intialize ring buffer */
void ringbuf_init(ringbuf_t *p, uint32_t capacity)
{
//	ASSERT(capacity <= RINGBUF_MAX_SIZE);

	p->capacity = capacity;
	p->mask = (p->capacity - 1);
	p->write = 0;
	p->read = 0;

	ringbuf_clear_overflow(p);
	ringbuf_clear_underflow(p);
}



/* Insert byte into ring buffer */
void ringbuf_insert(ringbuf_t *p, uint8_t data)
{
	if(ringbuf_full(p))
	{
		p->overflow = 1;
		return;
	}
	p->buffer[p->write++ & p->mask] = data;
}



/* Remove byte from ring buffer */
uint8_t ringbuf_remove(ringbuf_t *p)
{
	if(ringbuf_empty(p))
	{		
		p->underflow = 1;
		return 0xFF;
	}
	return p->buffer[p->read++ & p->mask];
}



/* Test if ring buffer is empty */
int ringbuf_empty(ringbuf_t *p)
{
	return (p->write == p->read);
}



/* Get size of available data to read from ring buffer */
uint8_t ringbuf_size(ringbuf_t *p)
{
	return p->write - p->read;
}


/* Check if ring buffer is full */
int ringbuf_full(ringbuf_t *p)
{
	return ringbuf_size(p) == p->capacity;
}

/* End */

