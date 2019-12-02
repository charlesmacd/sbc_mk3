
#ifndef _POST_H_
#define _POST_H_

#ifndef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "../hw_defs.hpp"

typedef struct
{
    volatile uint8_t *DATA;
} post_register_t;


class Post
{
    post_register_t reg;
    uint8_t state;
public:

    void initialize(void)
    {
        reg.DATA = (volatile uint8_t *)(POST_BASE);
    }
    void set(uint16_t value)
    {
        *reg.DATA = state = value;
    }

    uint8_t get(void)
    {
        return state;
    }
};


#ifndef __cplusplus
}; /* extern */
#endif

#endif /* _POST_H_ */