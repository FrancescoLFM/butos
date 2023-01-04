CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump
QEMU = qemu-system-x86_64

CFLAGS  = -ffreestanding -nostdlib -fno-pie -fno-stack-protector
CFLAGS += -Wall -Wextra -Werror
CFLAGS += -m32 -std=gnu99
CFLAGS += -O2 -g
CFLAGS += -I.

ASFLAGS = -march=i386 --32

LDSCRIPT = butos.ld

SOURCES_C = $(wildcard *.c **/*.c)
SOURCES_ASM = $(wildcard *.s **/*.s)

OBJECTS_C = $(SOURCES_C:.c=.o)
OBJECTS_ASM = $(SOURCES_ASM:.s=.o)

OBJECTS = $(OBJECTS_C) $(OBJECTS_ASM)
EXECUTABLE = butos
BINARY = butos.bin

DEBUG_FILE = debug.dbg

define color_text
	@echo -e "\033[$1m$2\033[0m"
endef

.PHONY=all
all: $(SOURCES_C) $(SOURCES_ASM) $(BINARY)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.s.o:
	$(AS) $(ASFLAGS) -c $< -o $@


$(EXECUTABLE): $(OBJECTS)
	$(LD) $^ -o $@ -T $(LDSCRIPT) --ignore-unresolved-symbol _GLOBAL_OFFSET_TABLE


$(BINARY): $(EXECUTABLE)
	$(call color_text,32,"[BUTOS] Compilazione terminata")
	$(OBJCOPY) --only-keep-debug $< $(DEBUG_FILE)
	$(OBJCOPY) -O binary -S $< $@
	$(call color_text,32,"[BUTOS] Generazione del binario terminata")


.PHONY=clean
clean:
	-rm -f $(OBJECTS)
	-rm -f $(EXECUTABLE)
	-rm -f $(BINARY)
	-rm -f $(DEBUG_FILE)

.PHONY=disass
disass:
	$(OBJDUMP) -mi386 -d $(EXECUTABLE)