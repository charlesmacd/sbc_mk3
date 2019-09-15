
#ifndef _CRT0_H_
#define _CRT0_H_


extern char _end; /* s24-cpua.ld */

extern void soft_reset(void);
extern void soft_reboot(void);
extern void hard_fault(void);
extern uint32_t get_sp(void);
extern uint32_t get_bp(void);

#endif /* _CRT0_H_ */