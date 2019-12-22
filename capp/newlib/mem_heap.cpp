/*
    File:
        mem_heap.c


    __end : Symbol in linker file placed after all text and data
            Note: Kernel area is flagged as no-load and placed after __end as to not increase the position.
            
*/
#include <stdlib.h>
#include "../sys_types.hpp"
#include "../sbc.hpp"

extern "C" {

extern char _end;
void *sbrk(int incr)
{
	static char *heap_end = NULL;
	char *prev_heap_end = NULL;

	/* If we haven't initialized hap_end to point to the end of the heap, do so now (highest address after end of loaded program)*/
	if(heap_end == NULL)
	{
		heap_end = &_end;
	}

	/* Take note of current heap end */
	prev_heap_end = heap_end;

	/* If adjustment of the heap collides with the stack, we're out of memory */
	if((uint32_t)(heap_end + incr) >= get_sp())
	{
		// collision
	}

	/* Adjust heap end */
	heap_end += incr;

	/* Return pointer to previous heap end */
	return (void *)prev_heap_end;
}

};

/* End */