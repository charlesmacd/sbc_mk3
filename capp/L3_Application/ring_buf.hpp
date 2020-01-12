#ifndef _RING_BUF_H_
#define _RING_BUF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "..\debug.h"


#define RINGBUF_MAX_SIZE		128

class RingBuffer
{
public:
	volatile uint8_t capacity;
	volatile uint8_t index_mask;
	volatile uint8_t write_index;
	volatile uint8_t read_index;
    volatile uint8_t count;
	volatile bool underflow;
	volatile bool overflow;
	volatile uint8_t buffer[RINGBUF_MAX_SIZE];
   

    /* Clear oveflow condition */
    void clear_overflow(void)
    {
        overflow = false;
    }

    /* Clear underflow condition */
    void clear_underflow(void)
    {
        underflow = false;
    }


    /* Intialize ring buffer */
    bool initialize(uint32_t buffer_capacity)
    {
        /* Validate capacity is a non-zero power-of-two */
        if(buffer_capacity == 0 || __builtin_popcount(buffer_capacity) != 1)
        {
            return false;
        }

        capacity = buffer_capacity;
        index_mask = (capacity - 1);
        write_index = 0;
        read_index = 0;
        count = 0;

        clear_overflow();
        clear_underflow();

        return true;
    }

    /* Insert byte into ring buffer */
    void insert(uint8_t data)
    {
        if(full())
        {
            overflow = true;
            // Count # of lost bytes here
            return;
        }
        buffer[write_index++ & index_mask] = data;
        ++count;
    }


    /* Look at next element to read; trigger underflow if empty */
    volatile uint8_t peek(void)
    {
        if(empty())
        {
            underflow = true;
            return 0xFF;
        }
        return buffer[read_index & index_mask];
    }


    /* Remove byte from ring buffer */
    volatile uint8_t remove(void)
    {
        uint8_t temp;

        if(empty())
        {		
            underflow = true;
            return 0xFF;
        }

        temp = buffer[read_index++ & index_mask];
        --count;

        return temp;
    }


    volatile bool empty(void)
    {
        return (count == 0);
    }

    volatile uint8_t used(void)
    {
        return count;
    }

    volatile uint8_t remaining(void)
    {
        return capacity - count;
    }


    /* Check if ring buffer is full */
    volatile int full(void)
    {
        return count == capacity;
    }
};


#ifdef __cplusplus
} /* extern */
#endif

#endif /* _RING_BUF_H_ */
