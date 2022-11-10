SHELL		= /bin/bash

CC			= gcc
CFLAGS		= -ffreestanding -m32 -O2 -std=gnu99 -Wall -Wextra -Werror -Wno-unused-parameter -fno-pie -fno-stack-protector -g
LD			= ld
AS			= as
ASFLAGS 	= -march=i386 --32

PROGDIR		= ..
LIBDIR		= $(PROGDIR)/libs
LIBCPUDIR	= $(LIBDIR)/cpu
SCRIPTDIR 	= ../$(PROGDIR)/scripts
CONFDIR		= ../$(PROGDIR)/conf

CFLAGS 	   += -I$(PROGDIR)/

OBJ			= prova.o $(LIBDIR)/print.o $(LIBDIR)/string.o $(LIBDIR)/pci.o $(LIBDIR)/kb.o $(LIBDIR)/array.o $(LIBCPUDIR)/idt.o $(LIBCPUDIR)/isr.o $(LIBCPUDIR)/pic.o
NAME		= butos
TARGET		= $(NAME).bin
LDSCRIPT	= $(NAME).ld
CONFIGURE	= $(SCRIPTDIR)/configure.sh

DBG			= gdb
DBGGEN		= $(SCRIPTDIR)/debug_gen.sh
DGBSYM		= $(NAME).out.debug
DGBSCR		= $(SCRIPTDIR)/butos.gdb

.PHONY=all
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBDIR)/%.o: $(LIBDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBCPUDIR)/%.o: $(LIBCPUDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBCPUDIR)/%.o: $(LIBCPUDIR)/%.s
	$(AS) $(ASFLAGS) $< -o $@

$(NAME).out: $(OBJ)
	$(LD) $^ -o $@ -T $(LDSCRIPT) --ignore-unresolved-symbol _GLOBAL_OFFSET_TABLE_ 

$(TARGET): $(NAME).out
	objcopy	-O binary -S $^ $@
	$(DBGGEN) $^
	$(CONFIGURE) $@ $(CONFDIR)
	rm $^

.PHONY=debug
debug: $(TARGET)
	make -C ../.. all
	qemu-system-x86_64 -s -S $(PROGDIR)/../bin/boot.bin &
	$(DBG) -q -s $(DGBSYM) -x $(DGBSCR)
	killall qemu-system-x86

.PHONY=disass
disass:
	make $(NAME).out
	objdump -mi386 -d $(NAME).out
	rm $(NAME).out

.PHONY=clean
clean:
	rm $(OBJ) $(TARGET) $(DGBSYM)

.PHONY=run
run:
	make -C $(PROGDIR)/.. run
