include ../toolchain.mk

NAME		= bootloader

SECTORS 	= $(shell printf "0x%X" $$(( $$(wc -c < ../src/butos.bin) / 512 + 1 )))
ASFLAGS		= -I./ --defsym SEC=$(SECTORS)

LDSCR		= $(NAME).ld


EXECUTABLE  = $(NAME).out
BINARY		= $(NAME).bin

SOURCES		:= $(wildcard *.s) $(wildcard libs/*.s)
OBJECTS		:= $(patsubst %.s,%.o,$(SOURCES))

.PHONY: all
all: $(BINARY)

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(call color_text,33,"[INIT] Linking del bootloader")
	$(LD) -T $(LDSCR) $^ -o $@

$(BINARY): $(EXECUTABLE)
	$(call color_text,33,"[INIT] Generazione del binario $(BINARY)")
	$(STRIP) --remove-section=.note.gnu.property $^
	$(OBJCOPY) -O binary -S $^ $@
	printf '\125\252' | dd of=$@ bs=1 seek=510 count=2 conv=notrunc
	$(MAKE) -C mbr/
	cat mbr/bootstrap.bin bootloader.bin > boot.bin

.PHONY: run
run:
	qemu-system-x86_64 -drive format=raw,file=$(BINARY)

.PHONY: clean
clean:
	-rm -f $(OBJECTS)
	-rm -f $(EXECUTABLE)
	-rm -f $(BINARY)
	$(MAKE) -C mbr/ clean

.PHONY: debug
debug:
	@echo "run `target remote localhost:123` inside a gbd instance"
	qemu-system-x86_64 -drive format=raw,file=$(BINARY) -s -S
