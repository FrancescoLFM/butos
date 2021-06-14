CC 		= gcc
CFLAGS	= -g -ffreestanding -Wall

NAME 	= test
OBJ 	= string.o test.o 
TARGET 	= $(NAME).elf

.PHONY=all
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) *.h $(OBJ) -o $@

.PHONY=clean
clean:
	rm *.o $(TARGET)