SHELL		= /bin/bash

CC			= gcc
CFLAGS		= -ffreestanding -m32 -O2 -std=gnu99 -Wall -Wextra -Werror -fno-pie -fno-stack-protector -g
LD			= ld
AS			= as
ASFLAGS 	= -march=i386 --32

# Cartelle
PROGDIR		= .
BINDIR		= $(PROGDIR)/butos
LIBDIR		= $(PROGDIR)/libs
LIBCPUDIR	= $(PROGDIR)/cpu
DRIVERSDIR  = $(PROGDIR)/drivers
SCRIPTDIR 	= ../scripts
CONFDIR		= ../conf

CFLAGS 	   += -I$(PROGDIR)/

# File oggetto
OBJ			= $(PROGDIR)/startk.o $(PROGDIR)/test.o
OBJ		   += $(LIBCPUDIR)/idt.o $(LIBCPUDIR)/isr.o $(LIBCPUDIR)/pic.o
OBJ		   += $(LIBDIR)/print.o $(LIBDIR)/string.o $(LIBDIR)/allocator.o $(LIBDIR)/alloc.o $(LIBDIR)/queue.o $(LIBDIR)/scan.o
OBJ		   += $(DRIVERSDIR)/pci.o $(DRIVERSDIR)/ata.o $(DRIVERSDIR)/keyboard.o $(DRIVERSDIR)/vga.o


NAME		= butos
TARGET		= $(BINDIR)/$(NAME).bin
LDSCRIPT	= $(PROGDIR)/$(NAME).ld
CONFIGURE	= $(SCRIPTDIR)/configure.sh

DBG			= gdb
DBGGEN		= $(SCRIPTDIR)/debug_gen.sh
DGBSYM		= $(PROGDIR)/$(NAME).out.debug
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
	@printf "\n[BUTOS] Compilazione terminata\n\n"
	-mkdir $(BINDIR)
	objcopy	-O binary -S $^ $@
	$(DBGGEN) $^
	$(CONFIGURE) $@ $(CONFDIR)
	@printf "\n[BUTOS] Generazione del sorgente terminata\n\n"
	rm $^

.PHONY=debug
debug:
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
