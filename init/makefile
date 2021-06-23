NAME		= boot

AS			= as

CONFDIR		= ../conf
SECTORS 	= 0x$(shell cat $(CONFDIR)/sectors.dat)
ASFLAGS		= -I./ --defsym SEC=$(SECTORS)

LD			= ld
LDSCR		= $(NAME).ld

LIBDIR		= libs
TARGET		= $(NAME).bin
OBJ			= init.o $(LIBDIR)/gdt.o $(LIBDIR)/write.o $(LIBDIR)/diskout.o $(LIBDIR)/bioslib.o
.PHONY=all
all: $(TARGET)

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

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
	rm $(LIBDIR)/*.o *.o $(TARGET)