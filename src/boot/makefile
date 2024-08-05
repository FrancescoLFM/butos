CC = gcc
OBJCOPY = objcopy
OBJDUMP = objdump

FLAGS = -Wall -Wextra -Werror
FLAGS += -fno-pie -m32
FLAGS += -g
FLAGS += -I../.

CFLAGS = $(FLAGS)
CFLAGS += -ffreestanding -nostdlib -fno-stack-protector -std=c99 -mgeneral-regs-only

LDSCRIPT = boot.ld

SOURCES_C = startk.c ../cpu/paging.c
SOURCES_ASM = ../cpu/apaging.s

OBJECTS_C = $(SOURCES_C:.c=.o)
OBJECTS_ASM = $(SOURCES_ASM:.s=.o)

OBJECTS = $(OBJECTS_C) $(OBJECTS_ASM)
EXECUTABLE = boot
BINARY = boot.bin

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
	$(call color_text,32,"[BOOTLOADER] Compilazione terminata")
	$(OBJCOPY) -O binary -S $< $@
	$(OBJCOPY) --only-keep-debug $< $(DEBUG_FILE)
	$(call color_text,32,"[BOOTLOADER] Generazione del binario terminata")


.PHONY=clean
clean:
	-rm -f $(OBJECTS)
	-rm -f $(EXECUTABLE)
	-rm -f $(BINARY)
	-rm -f $(DEBUG_FILE)

.PHONY=disass
disass:
	$(OBJDUMP) -mi386 -d $(EXECUTABLE)
