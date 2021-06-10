CC		= gcc
CFLAGS	= -ffreestanding -m32 -Wall -Wextra -fno-pie
LD		= ld

OBJ		= main.o asm.o vga.o
NAME	= lanfredi
TARGET	= $(NAME).bin
SCRIPT	= $(NAME).ld

.PHONY=all
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(NAME).out: $(OBJ)
	$(LD) $^ -o $@ -T $(SCRIPT) --ignore-unresolved-symbol _GLOBAL_OFFSET_TABLE_

$(TARGET): $(NAME).out
	objcopy	-O binary -S $^ $@
	./configure.sh $@
	rm $^

.PHONY=clean
clean:
	rm *.o $(TARGET)