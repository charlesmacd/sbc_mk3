#
# Makefile for SBC RAM-loadable program
#

CLIENT	=	sbc_mk3
LDADDR	=	0x110000

CPUTYPE	=	68000
CPUDIR	=	m$(CPUTYPE)
SGDIR	=	c:\SysGCC
SGVER	=	4.8.0
OPT		=	-O0
LSCRIPT	=	platform/$(CPUTYPE)/linker_ram.ld

CC	=	m68k-elf-g++
CCC =	m68k-elf-gcc

AS	=	m68k-elf-as
LD	=	m68k-elf-ld
OC	=	m68k-elf-objcopy

CINLCUDES =  -Iperipheral -Iutility -Iapplication -Iplatform

ASFLAGS	=	-m$(CPUTYPE) --register-prefix-optional
#LDFLAGS	=	--oformat binary -T $(LSCRIPT)
LDFLAGS	=	--oformat elf32-m68k -T $(LSCRIPT)
CCFLAGS	=	-march=$(CPUTYPE) -mcpu=$(CPUTYPE) -mtune=$(CPUTYPE) -msoft-float $(OPT) -g -fno-builtin \
			$(CINCLUDES) -Wfatal-errors -fno-exceptions -Wa,--register-prefix-optional
LIBS	=	$(SGDIR)\m68k-elf\m68k-elf\lib\m68000\libc.a
LIBS	+=	$(SGDIR)\m68k-elf\lib\gcc\m68k-elf\$(SGVER)\$(CPUDIR)\libgcc.a
LIBS	+=	$(SGDIR)\m68k-elf\m68k-elf\lib\softfp\libm.a 
LIBS	+=	$(SGDIR)\m68k-elf\m68k-elf\lib\softfp\libg.a


# Third party libraries
OBJ_3RD		=	\
	obj/lzgmini_68k.o

# Utilities
OBJ_UTIL=	\
	obj/ringbuf.o

# Core files
OBJ_CORE	=	\
	obj/crt0.o	\
	obj/main.o	\
	obj/mem_heap.o \
	obj/usb.o	\
	obj/uart.o	\
	obj/timer.o	\
	obj/comms.o	\
	obj/printf.o	\
	obj/sbc.o	\
	obj/pic_flash.o	\
	obj/cpu_asm.o \
	obj/comms_asm.o	\
	obj/uart_asm.o \
	obj/target_asm.o	\
	obj/printf_asm.o	\
	obj/syscall_asm.o	\
	obj/interrupt_controller.o	\
	obj/target.o \
	obj/post.o

OBJ_APP	=	\
	obj/cli.o	\
	obj/cli_cmd.o

OBJ	=	$(OBJ_CORE) $(OBJ_UTIL) $(OBJ_3RD) $(OBJ_APP)

AS_INC	=	macros.inc	\
		usb.inc		

#		obj/ide.o	\
#		obj/usb.o	\
#		obj/comms.o	\
#		obj/util_asm.o

CLEANFILES	=	$(ELF) $(EXE) $(OBJ)

APPNAME	=	app
ELF	=	$(APPNAME).elf
EXE	=	$(APPNAME).bin

all	:	$(EXE)

$(EXE)	:	$(OBJ)
		$(LD) $(OBJ) $(LDFLAGS) -o $(ELF) $(LIBS)
		$(OC) -O binary $(ELF) $(EXE)
	        
obj/%.o : 	%.s $(AS_INC) 
		$(AS) $< -o $@ $(ASFLAGS)
	        
obj/%.o : 	peripheral/%.s $(AS_INC) 
		$(AS) $< -o $@ $(ASFLAGS)
	        
obj/%.o : third_party/%.s $(AS_INC) 
		$(AS) $< -o $@ $(ASFLAGS)
	        
obj/%.o : 	%.cpp %.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : 	utility/%.cpp utility/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : 	utility/%.c utility/%.h
		$(CCC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : application/%.cpp application/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)

obj/%.o : platform/%.cpp platform/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : peripheral/%.cpp peripheral/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)
	        

.PHONY	:	echo
echo	:
		$(CLIENT) -e

.PHONY	:	reset
reset	:
		$(CLIENT) -r

.PHONY	:	run
run	:	$(EXE)
		$(CLIENT) -r
		$(CLIENT) -x $(EXE) $(LDADDR)

.PHONY	:	clean
clean	:	        
		rm -f $(CLEANFILES)

.PHONY	:	makedir
makedir :
		mkdir obj

.PHONY	:	dasm
dasm	:
		unidasm $(EXE) -arch m$(CPUTYPE)

.PHONY  : kernel
kernel  :
		m68k-elf-objdump -t $(ELF) | grep KERNEL

#
# end of makefile
#
