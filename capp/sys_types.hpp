#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

/* Place a variable in the kernel memory section */
/* NOTE: Must not be initialized */
#define KERNEL		__attribute__((section(".KERNELVARS")))

#endif /* _SYS_TYPES_H_ */
