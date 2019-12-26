
#ifndef _CPU_ASM_H_
#define _CPU_ASM_H_

/* Macros */
#define RESET()						__asm__ __volatile__("reset\n")
#define ENTER_CRITICAL_SECTION()	__asm__ __volatile__("move.w\t#0x2700, %sr\n")
#define EXIT_CRITICAL_SECTION()		__asm__ __volatile__("move.w\t#0x2000, %sr\n")
#define WAIT_IRQ_ANY()				__asm__ __volatile__("stop\t#0x2000\n")

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
uint32_t __ffs32(uint32_t value);
uint32_t __popcount32(uint32_t value);
uint32_t __atomic_tas(volatile uint8_t *ptr);
uint32_t get_sp(void);
uint32_t get_bp(void);
uint16_t get_sr(void);
uint16_t get_ccr(void);
uint16_t get_ipl(void);
void set_ipl(uint32_t level);
void set_sr(uint16_t sr);
void trigger_hard_fault(void);

#ifdef __cplusplus
}
#endif

#endif /* _CPU_ASM_H_ */

