NAME	= boot

AS		= as
LD		= ld
LDSCR	= $(NAME).ld

TARGET	= $(NAME).bin
OBJ		= init.o write.o read.o diskout.o
ENTRY	= init

.PHONY=all
all: $(TARGET)

%.o: %.s
	$(AS) $< -o $@

$(TARGET): $(OBJ)
	$(LD) -T $(LDSCR) $^ -o $(NAME).out
	strip --remove-section=.note.gnu.property $(NAME).out
	objcopy -O binary -S $(NAME).out $@
	dd seek=1 bs=512 count=1 if=/dev/zero of=$(NAME).bin
	rm $(NAME).out

.PHONY=clean
clean:
	rm *.o $(TARGET)

.PHONY=run
run:
	qemu-system-x86_64 $(TARGET)