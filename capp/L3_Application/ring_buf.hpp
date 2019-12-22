#ifndef _RING_BUF_H_
#define _RING_BUF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "../L0_Platform/cpu_asm.hpp"

#define RINGBUF_MAX_SIZE		128

class RingBuffer
{
public:
	uint8_t capacity;
	uint8_t index_mask;
	uint8_t write_index;
	uint8_t read_index;
    uint8_t count;
	bool underflow;
	bool overflow;
	uint8_t buffer[RINGBUF_MAX_SIZE];
   

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
    bool initialize(uint32_t capacity)
    {
        /* Validate capacity is a non-zero power-of-two */
        if(capacity == 0 || __popcount32(capacity) != 1)
        {
            return false;
        }

        this->capacity = capacity;
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
    uint8_t peek(void)
    {
        if(empty())
        {
            underflow = true;
            return 0xFF;
        }
        return buffer[read_index & index_mask];
    }


    /* Remove byte from ring buffer */
    uint8_t remove(void)
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


    /* Test if ring buffer is empty */
    int empty(void)
    {
        return (count == 0);
    }

    uint8_t used(void)
    {
        return count;
    }

    uint8_t remaining(void)
    {
        return capacity - count;
    }


    /* Check if ring buffer is full */
    int full(void)
    {
        return count == capacity;
    }
};


#ifdef __cplusplus
}; /* extern */
#endif

#endif /* _RING_BUF_H_ */
