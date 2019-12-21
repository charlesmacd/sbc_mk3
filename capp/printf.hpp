
#ifndef _PRINTF_H_
#define _PRINTF_H_

#ifdef __cplusplus
extern "C" {
#endif

//extern "C" void *sbrk(int incr);

extern "C" int puts(const char *msg);
extern "C" int printf(const char *fmt, ...);

#ifdef __cplusplus
};
#endif

#endif /* _PRINTF_H_ */

