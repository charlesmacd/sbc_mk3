
#ifndef _CRT0_H_
#define _CRT0_H_

#ifdef __cplusplus
extern "C" {
#endif

void soft_reset(void);
void soft_reboot(void);

#ifdef __cplusplus
}
#endif

#endif /* _CRT0_H_ */