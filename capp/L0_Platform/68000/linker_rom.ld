
/* Linker script for application in ROM (Flash) */

OUTPUT_ARCH(m68k)
SEARCH_DIR(.)
/*GROUP(-lbcc -lc -lgcc)*/
__DYNAMIC  =  0;

MEMORY
{
  flash  : ORIGIN = 0x00000000, LENGTH = 0x00100000
	ram    : ORIGIN = 0x00100000, LENGTH = 0x00100000
	kernel : ORIGIN = 0x00100800, LENGTH = 0x00000800
}

PROVIDE (__stack = 0x00110000);

SECTIONS
{
  .text 0x00000000:
  {
  	_text_start = .;
    *(.text .text.*)
    . = ALIGN(0x4);
     __CTOR_LIST__ = .;
    LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
    *(.ctors)
    LONG(0)
    __CTOR_END__ = .;
    __DTOR_LIST__ = .;
    LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
    *(.dtors)
     LONG(0)
    __DTOR_END__ = .;
    *(.rodata .rodata.*)
    *(.gcc_except_table .gcc_except_table.*)

    . = ALIGN(0x4);
    __INIT_SECTION__ = . ;
    _init = .;
    *(.init)
    SHORT (0x4E75)	/* rts */

    __FINI_SECTION__ = . ;
    _fini = .;
    *(.fini)
    SHORT (0x4E75)	/* rts */

    _etext = .;
    *(.lit)
  } > flash
  _stext = SIZEOF (.text);

  .data :
  AT ( ADDR (.text) + SIZEOF (.text) )
  {
    . = ALIGN(0x4);
    LONG(0xdeadbeef)
  	_data_start = .;
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.*)
    _edata = .;
  } > flash
  _sdata = SIZEOF (.data);

  .bss :
  AT ( ADDR (.text) + SIZEOF (.text) + SIZEOF(.data) )
  {
    . = ALIGN(0x4);
    LONG(0xdeadbeef)
  	_bss_start = .;
    _start = . ;
    *(.shbss)
    *(.bss .bss.*)
    *(COMMON)
    _bend = . ;
  } > ram
  _sbss = SIZEOF (.bss);

  .stab 0 (NOLOAD) :
  {
    *(.stab)
  }

  .stabstr 0 (NOLOAD) :
  {
    *(.stabstr)
  }

  .eh_frame 0 (NOLOAD) :
  {
    *(.eh_frame)
  }
  . = ALIGN(0x10);

  _end = .;


  .KERNELSECTION 0x00100800 (NOLOAD) :
  {
    _kernel_start = .;
    KEEP(*(.KERNELVARS))
    _kernel_end = .;
  } > kernel
  _skernel = SIZEOF (.KERNELSECTION);







}
