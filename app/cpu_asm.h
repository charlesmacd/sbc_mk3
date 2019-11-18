
#ifndef _CPU_ASM_H_
#define _CPU_ASM_H_

extern uint32_t get_sp(void);
extern uint32_t get_bp(void);
extern uint16_t get_sr(void);
extern uint16_t get_ccr(void);
extern void set_sr(uint16_t sr);
extern void trigger_hard_fault(void);

#endif /* _CPU_ASM_H_ */

