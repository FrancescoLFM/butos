NAME		= bootloader

AS			= as

SECTORS 	= $(shell printf "0x%X" $(shell echo $(shell stat --format=%s ../src/butos.bin) / 512 + 1 | bc))
ASFLAGS		= -I./ --defsym SEC=$(SECTORS)

LD			= ld
LDSCR		= $(NAME).ld


EXECUTABLE  = $(NAME).out
BINARY		= $(NAME).bin

SOURCES		:= $(wildcard *.s) $(wildcard libs/*.s)
OBJECTS		:= $(patsubst %.s,%.o,$(SOURCES))

define color_text
	@echo -e "\033[$1m$2\033[0m"
endef

.PHONY: all
all: $(BINARY)

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(call color_text,33,"[INIT] Linking del bootloader")
	$(LD) -T $(LDSCR) $^ -o $@

$(BINARY): $(EXECUTABLE)
	$(call color_text,33,"[INIT] Generazione del binario $(BINARY)")
	strip --remove-section=.note.gnu.property $^
	objcopy -O binary -S $^ $@
	/usr/bin/printf "\x55\xaa" | dd of=$@ bs=1 seek=510 count=2 conv=notrunc
	make -C mbr/
	cat mbr/bootstrap.bin bootloader.bin > boot.bin

.PHONY: run
run:
	qemu-system-x86_64 -drive format=raw,file=$(BINARY)

.PHONY: clean
clean:
	-rm -f $(OBJECTS)
	-rm -f $(EXECUTABLE)
	-rm -f $(BINARY)
	make -C mbr/ clean

.PHONY: debug
debug:
	@echo "run `target remote localhost:123` inside a gbd instance"
	qemu-system-x86_64 -drive format=raw,file=$(BINARY) -s -S
