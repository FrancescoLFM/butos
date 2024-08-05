CC = gcc
OBJCOPY = objcopy
OBJDUMP = objdump
QEMU = qemu-system-x86_64

FLAGS = -Wall -Wextra -Werror
FLAGS += -fno-pie -m32
FLAGS += -g
FLAGS += -I.

CFLAGS = $(FLAGS)
CFLAGS += -ffreestanding -nostdlib -fno-stack-protector -std=c99 -mgeneral-regs-only

LDSCRIPT = butos.ld

SOURCES_C = $(wildcard *.c cpu/*.c drivers/*.c fs/*.c libs/*.c fs/fat32/*.c)
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
	$(CC) $(FLAGS) -c $< -o $@


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ -T $(LDSCRIPT)


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
