/*

  100000-1FFFFF : Work RAM (1MB)
  180000-1FFFFF : 512K for loaded program, heap starts past end
  100000-1007FF : 256 x [jmp $mem32 nop]
  100800-107FFF : Stack area (~30K)
  108000        : Stack pointer (MSP, USP)
  180000        : __stack, .text, .data, .bss

  We currenty have var base in last 256 bytes (!)


*/

OUTPUT_ARCH(m68k)
SEARCH_DIR(.)
/*GROUP(-lbcc -lc -lgcc)*/
__DYNAMIC  =  0;

/*
 * Setup the memory map of the SEGA Genesis.
 * stack grows down from high memory.
 *
 * The memory map look like this:
 * +--------------------+ <- low memory
 * | .text              |
 * |        _etext      |
 * |        ctor list   | the ctor and dtor lists are for
 * |        dtor list   | C++ support
 * +--------------------+
 * .                    .
 * .                    .
 * .                    .
 * +--------------------+ <- 0x00FF0000
 * | .data              | initialized data goes here
 * |        _data       |
 * |        _edata      |
 * +--------------------+
 * | .bss               |
 * |        _bstart     | start of bss, cleared by crt0
 * |        _bend       | start of heap, used by sbrk()
 * +--------------------+
 * .                    .
 * .                    .
 * .                    .
 * |        __stack     | top of stack
 * +--------------------+ <- 0x01000000
 */
MEMORY
{
	ram  : ORIGIN = 0x00100000, LENGTH = 0x00100000
	kernel : ORIGIN = 0x00100800, LENGTH = 0x00000800
}

/*
 * allocate the stack to be at the top of memory, since the stack
 * grows down
 */

PROVIDE (__stack = 0x00110000);


SECTIONS
{
  .text 0x00110000:
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
  } > ram
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
  } > ram
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
