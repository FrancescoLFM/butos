NAME	= boot

AS		= as
LD		= ld
LDSCR	= $(NAME).ld

HOME	= ..
TARGET	= $(NAME).bin
LIBDIR	= $(HOME)/libs
OBJ		= init.o gdt.o $(LIBDIR)/write.o $(LIBDIR)/diskout.o $(LIBDIR)/bioslib.o
ENTRY	= init

.PHONY=all
all: $(TARGET)

%.o: %.s
	$(AS) $< -o $@

$(NAME).out: $(OBJ)
	$(LD) -T $(LDSCR) $^ -o $@

$(TARGET): $(NAME).out
	strip --remove-section=.note.gnu.property $^
	objcopy -O binary -S $^ $@
	printf "\x55\xaa" | dd of=$@ bs=1 seek=510 count=2 conv=notrunc 
	rm $^

.PHONY=run
run: $(TARGET)
	qemu-system-x86_64 $^

.PHONY=clean
clean:
	rm $(OBJ) $(TARGET)