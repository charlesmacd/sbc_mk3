#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

/* Place a variable in the kernel memory section */
/* NOTE: Must not be initialized */
#define KERNEL		__attribute__((section(".KERNELVARS")))

// From ARM CMSIS

#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions */
#define     __IO    volatile             /*!< Defines 'read / write' permissions */

/* following defines should be used for structure members */
#define     __IM     volatile const      /*! Defines 'read only' structure member permissions */
#define     __OM     volatile            /*! Defines 'write only' structure member permissions */
#define     __IOM    volatile            /*! Defines 'read / write' structure member permissions */


/* Pad a structure in 2-byte increments */
#define STRUCT_PADDING(name, size, used)		uint16_t RESERVED##name[(size/2)-used]


#endif /* _SYS_TYPES_H_ */

