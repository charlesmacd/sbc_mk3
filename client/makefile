# makefile

CC	=	g++
LDFLAGS	=	-s
FLAGS	=	-I. -Wno-write-strings
LIBS	=	ftd2xx.lib

# Emulation core
OBJ	=       obj/main.o	\
		obj/usb.o

EXE	=	sbc_mk3.exe

all	:	$(EXE)

$(EXE)	:	$(OBJ)
		$(CC) -o $(EXE) $(OBJ) $(LIBS) $(LDFLAGS)
	        
obj/%.o : 	%.cpp %.h
		$(CC) -c $< -o $@ $(FLAGS)

.PHONY:	clean
clean:
	rm -f $(OBJ)
	rm -f $(EXE)
	        
# end of makefile
