NAME		= boot

AS			= as

CONFDIR		= ../conf
SECTORS 	= 0x$(shell cat $(CONFDIR)/sectors.dat)
ASFLAGS		= -I./ --defsym SEC=$(SECTORS)

LD			= ld
LDSCR		= $(NAME).ld

LIBDIR		= libs
TARGET		= $(NAME).bin
OBJ			= init.o
OBJ		   += $(LIBDIR)/gdt.o $(LIBDIR)/write.o $(LIBDIR)/diskout.o $(LIBDIR)/bioslib.o

.PHONY=all
all:
	printf "\n[INIT] Assemblaggio del bootloader\n\n"
	make $(TARGET)

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(NAME).out: $(OBJ)
	@printf "\n[INIT] Linking del bootloader\n\n"
	$(LD) -T $(LDSCR) $^ -o $@

$(TARGET): $(NAME).out
	@printf "\n[INIT] Generazione dell'eseguibile $(TARGET)\n\n"
	strip --remove-section=.note.gnu.property $^
	objcopy -O binary -S $^ $@
	printf "\x55\xaa" | dd of=$@ bs=1 seek=510 count=2 conv=notrunc 
	rm $^

.PHONY=run
run:
	qemu-system-x86_64 $(TARGET)

.PHONY=clean
clean:
	rm $(LIBDIR)/*.o *.o $(TARGET)