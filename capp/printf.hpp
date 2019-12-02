
#ifndef _PRINTF_H_
#define _PRINTF_H_

#ifdef __cplusplus
extern "C" {
#endif

void *sbrk(int incr);
int printf(const char *fmt, ...);

#ifdef __cplusplus
};
#endif

#endif /* _PRINTF_H_ */

