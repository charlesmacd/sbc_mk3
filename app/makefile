# Makefile for SBC RAM-loadable program

CLIENT	=	sbc_mk3
LDADDR	=	0x110000

CPUTYPE	=	68000
CPUDIR	=	m$(CPUTYPE)
SGDIR	=	c:\SysGCC
SGVER	=	4.8.0
OPT	=	-O0
LSCRIPT	=	sbc_ram.ld

CC	=	m68k-elf-gcc
AS	=	m68k-elf-as
LD	=	m68k-elf-ld
OC	=	m68k-elf-objcopy

ASFLAGS	=	-m$(CPUTYPE) --register-prefix-optional
#LDFLAGS	=	--oformat binary -T $(LSCRIPT)
LDFLAGS	=	--oformat elf32-m68k -T $(LSCRIPT)
CCFLAGS	=	-std=c99 -march=$(CPUTYPE) -mcpu=$(CPUTYPE) -mtune=$(CPUTYPE) -msoft-float $(OPT) -g -fno-builtin
LIBS	=	$(SGDIR)\m68k-elf\m68k-elf\lib\m68000\libc.a
LIBS	+=	$(SGDIR)\m68k-elf\lib\gcc\m68k-elf\$(SGVER)\$(CPUDIR)\libgcc.a
LIBS	+=	$(SGDIR)\m68k-elf\m68k-elf\lib\softfp\libm.a 
LIBS	+=	$(SGDIR)\m68k-elf\m68k-elf\lib\softfp\libg.a

# Emulation core
OBJ	=       obj/crt0.o	\
		obj/main.o	\
		obj/usb.o	\
		obj/uart.o	\
		obj/timer.o	\
		obj/comms.o	\
		obj/printf.o	\
		obj/ringbuf.o	\
		obj/cli.o	\
		obj/cli_cmd.o	\
		obj/sbc.o	\
		obj/pic_flash.o

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
	        
obj/%.o : 	%.c %.h
		$(CC) -c $< -o $@ $(CCFLAGS)
	        

.PHONY	:	echo
echo	:
		$(CLIENT) -e

.PHONY	:	reset
reset	:
		$(CLIENT) -r

.PHONY	:	run
run	:	
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
#
# end of makefile
#
