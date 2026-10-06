include ../../toolchain.mk

NAME		= bootstrap

LDSCR		= $(NAME).ld


EXECUTABLE  = $(NAME).out
BINARY		= $(NAME).bin

SOURCES		:= $(wildcard *.s) $(wildcard libs/*.s)
OBJECTS		:= $(patsubst %.s,%.o,$(SOURCES))

.PHONY: all
all: $(BINARY)

%.o: %.s
	$(AS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(call color_text,33,"[INIT] Linking del bootstrap")
	$(LD) -T $(LDSCR) $^ -o $@

$(BINARY): $(EXECUTABLE)
	$(call color_text,33,"[INIT] Generazione del binario $(BINARY)")
	$(STRIP) --remove-section=.note.gnu.property $^
	$(OBJCOPY) -O binary -S $^ $@
	printf '\125\252' | dd of=$@ bs=1 seek=510 count=2 conv=notrunc

.PHONY: run
run:
	qemu-system-x86_64 -drive format=raw,file=$(BINARY)

.PHONY: clean
clean:
	-rm -f $(OBJECTS)
	-rm -f $(EXECUTABLE)
	-rm -f $(BINARY)

