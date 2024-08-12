CC = gcc
OBJCOPY = objcopy
OBJDUMP = objdump
QEMU = qemu-system-x86_64

BINDIR = ../../bin

FLAGS = -Wall -Wextra -Werror
FLAGS += -fno-pie -m32
FLAGS += -g
FLAGS += -I.

CFLAGS = $(FLAGS)
CFLAGS += -ffreestanding -nostdlib -fno-stack-protector -std=c99 -mgeneral-regs-only

LDSCRIPT = example.ld

SOURCES_C = $(wildcard *.c)
OBJECTS = $(SOURCES_C:.c=.o)
EXECUTABLE = $(BINDIR)/example

define color_text
	@echo -e "\033[$1m$2\033[0m"
endef

.PHONY=all
all: $(SOURCES_C) $(SOURCES_ASM) $(EXECUTABLE)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ -T $(LDSCRIPT)

.PHONY=clean
clean:
	-rm -f $(OBJECTS)
	-rm -f $(EXECUTABLE)

.PHONY=disass
disass:
	$(OBJDUMP) -mi386 -d $(EXECUTABLE)
