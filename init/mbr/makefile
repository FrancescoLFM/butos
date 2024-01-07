NAME		= bootstrap

AS			= as

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
	$(AS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(call color_text,33,"[INIT] Linking del bootstrap")
	$(LD) -T $(LDSCR) $^ -o $@

$(BINARY): $(EXECUTABLE)
	$(call color_text,33,"[INIT] Generazione del binario $(BINARY)")
	strip --remove-section=.note.gnu.property $^
	objcopy -O binary -S $^ $@
	printf "\x55\xaa" | dd of=$@ bs=1 seek=510 count=2 conv=notrunc

.PHONY: run
run:
	qemu-system-x86_64 -drive format=raw,file=$(BINARY)

.PHONY: clean
clean:
	-rm -f $(OBJECTS)
	-rm -f $(EXECUTABLE)
	-rm -f $(BINARY)

