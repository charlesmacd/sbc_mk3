#
# Makefile for SBC RAM-loadable program
#

CLIENT	=	sbc_mk3
LDADDR	=	0x110000

CPUTYPE	=	68000
CPUDIR	=	m$(CPUTYPE)
SGVER	=	8.2.0
SGDIR	=	c:\outrun\outrun-gcc-$(SGVER)_mingw\gcc-$(SGVER)
OPT		=	-O0
LSCRIPT	=	L0_Platform/$(CPUTYPE)/linker_ram.ld

CC	=	m68k-elf-g++
CCC =	m68k-elf-gcc

AS	=	m68k-elf-as
LD	=	m68k-elf-ld
OC	=	m68k-elf-objcopy

CINLCUDES =  -Iperipheral -Iutility -Iapplication -Iplatform
CPPSTD    =	 c++2a

ASFLAGS	=	-m$(CPUTYPE) --register-prefix-optional
#LDFLAGS	=	--oformat binary -T $(LSCRIPT) 
LDFLAGS	=	--oformat elf32-m68k -T $(LSCRIPT)
# removed -no-rtti
CCFLAGS	=	-march=$(CPUTYPE) -mcpu=$(CPUTYPE) -mtune=$(CPUTYPE) -msoft-float $(OPT) -g -fno-builtin \
			$(CINCLUDES) -Wfatal-errors -fno-exceptions -Wa,--register-prefix-optional -std=$(CPPSTD) -fno-rtti
##CCFLAGS +=	-pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual 
LIBS	=	$(SGDIR)\m68k-elf\lib\libc.a
LIBS	+=	$(SGDIR)\m68k-elf\lib\libm.a 
LIBS	+=	$(SGDIR)\m68k-elf\lib\libg.a
LIBS	+=	$(SGDIR)\lib\gcc\m68k-elf\$(SGVER)\libgcc.a


# Third party libraries
OBJ_3RD		=	\
	obj/lzgmini_68k.o

# Utilities
OBJ_UTIL=	\
	obj/ring_buf.o


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
	obj/interrupt_controller.o	\
	obj/system_controller.o \
	obj/pit.o	\
	obj/mutex.o	\
	obj/newlib.o	\
	obj/cpu_680x0.o	\
	obj/app.o

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
	        
	        
obj/%.o : third_party/%.s $(AS_INC) 
		$(AS) $< -o $@ $(ASFLAGS)
	        
obj/%.o : 	%.cpp %.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : 	newlib/%.cpp newlib/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : 	utility/%.cpp utility/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : 	utility/%.c utility/%.h
		$(CCC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : platform/%.cpp platform/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)

# L3_Application

obj/%.o : L3_Application/%.cpp L3_Application/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)

obj/%.o : 	L3_Application/%.c L3_Application/%.h
		$(CCC) -c $< -o $@ $(CCFLAGS)

# L1_Peripheral	        

obj/%.o : L1_Peripheral/%.cpp L1_Peripheral/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : L1_Peripheral/%.s $(AS_INC) 
		$(AS) $< -o $@ $(ASFLAGS)

# L0_Platform

obj/%.o : L0_Platform/%.cpp L0_Platform/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : L0_Platform/%.s $(AS_INC) 
		$(AS) $< -o $@ $(ASFLAGS)

obj/%.o : L0_Platform/68000/%.cpp L0_Platform/68000/%.hpp
		$(CC) -c $< -o $@ $(CCFLAGS)
	        
obj/%.o : L0_Platform/68000/%.s $(AS_INC) 
		$(AS) $< -o $@ $(ASFLAGS)

# Phony targets

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

.PHONY	:	rund
rund	:	$(EXE)
		$(CLIENT) -r
		$(CLIENT) -c $(EXE) $(LDADDR)

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

.PHONY  : syms
syms  	:
		m68k-elf-objdump -t $(ELF)

.PHONY  : list
list  	:
		m68k-elf-objdump -t $(ELF) --disassemble --all-headers --source --demangle --wide

#
# end of makefile
#
