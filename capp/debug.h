
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void debug_puts(const char *str);
void debug_putch(char ch);
void debug_printhexl(uint32_t value);

#ifdef __cplusplus
};
#endif


#endif /* _DEBUG_H_ */