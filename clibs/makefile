CC 		= gcc
CFLAGS	= -g -ffreestanding -m32 -Wall -Wextra -fno-pie

OBJ 	= string.o asm.o vga.o

.PHONY=all
all: $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY=clean
clean:
	rm *.o