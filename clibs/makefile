CC 		= gcc
CFLAGS	= -g -ffreestanding -Wall

NAME 	= test
OBJ 	= string.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY=clean
clean:
	rm *.o