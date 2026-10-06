include toolchain.mk

INITDIR	= init
ISODIR	= src

INIT  	= $(INITDIR)/boot.bin
ISO		= $(ISODIR)/butos.bin

BINDIR	= bin
BINFILE	= $(BINDIR)/boot.bin

BOOTDIR = $(ISODIR)/boot/
BOOTFILE = $(BOOTDIR)/boot.bin

QEMUDIR = qemu
IMG		= $(QEMUDIR)/vhdd.img
PART	= $(QEMUDIR)/fat_part.img
TARGET	= $(IMG)
SIZE	= 200K
FORMAT  = raw
VMARGS	= -device piix3-ide,id=ide -drive id=disk,file=$(IMG),format=$(FORMAT),if=none -device ide-hd,drive=disk,bus=ide.0 -m 2G

DBG    ?= gdb
DBGSYM  = src/butos
DBGSCR  = scripts/butos.gdb

PROGDIR = programs
PROGBIN = $(PROGDIR)/bin/*

.PHONY=all
all:
	$(call color_text,91,"[MAKE] Compilazione del bootloader butos")
	$(MAKE) -C $(BOOTDIR)
	$(call color_text,91,"[MAKE] Compilazione del kernel butos")
	$(MAKE) -C $(ISODIR)
	$(call color_text,91,"[MAKE] Compilazione del bootloader in real mode")
	$(MAKE) -C $(INITDIR)
	$(MAKE) $(TARGET)
	$(call color_text,91,"[MAKE] Creazione della partizione FAT32")
	rm -f $(PART)
	mkfs.fat -F 32 --mbr=y -C $(PART) 1000
	$(call color_text,91,"[MAKE] Compilazione degli eseguibili di base")
	$(MAKE) -C $(PROGDIR)
	$(call color_text,91,"[MAKE] Copia degli eseguibili nella partizione")
	mcopy -o -i $(PART) $(PROGBIN) ::/
	dd if=$(PART) of=$(IMG) seek=203 

$(TARGET): $(BINFILE)
	$(call color_text,91,"[MAKE] Generazione del disco avviabile")
	-mkdir $(QEMUDIR)
	qemu-img create -f $(FORMAT) $@ $(SIZE)
	dd if=$^ of=$@ conv=notrunc


# The kernel lives between sector 66 and the FAT partition at sector 203
KERNEL_MAX_SIZE = $(shell echo $$(( (203 - 66) * 512 )))

$(BINFILE): $(ISO) $(INIT)
	@test $$(wc -c < $(ISO)) -le $(KERNEL_MAX_SIZE) || \
		{ echo "Kernel too big: $$(wc -c < $(ISO)) > $(KERNEL_MAX_SIZE) bytes"; exit 1; }
	$(call color_text,91,"[MAKE] Generazione dell eseguibile complessivo")
	-mkdir bin
	dd seek=0 bs=512 count=2 conv=notrunc if=$(INIT) of=$@
	dd seek=2 bs=512 conv=notrunc if=$(BOOTFILE) of=$@
	dd seek=66 bs=512 conv=notrunc if=$(ISO) of=$@

.PHONY=silent
silent:
	@make -s 2>/dev/null

.PHONY=clean
clean:
	$(call color_text,91,"[MAKE] Pulizia dei file di compilazione nel kernel bootloader")
	$(MAKE) -C $(BOOTDIR) clean
	$(call color_text,91,"[MAKE] Pulizia dei file di compilazione nel kernel")
	$(MAKE) -C $(ISODIR) clean
	$(call color_text,91,"[MAKE] Pulizia dei file di compilazione nel bootloader")
	$(MAKE) -C $(INITDIR) clean
	$(call color_text,91,"[MAKE] Rimozione del disco generato")
	rm $(TARGET)
	rm $(BINFILE)
	$(MAKE) -C $(PROGDIR) clean

.PHONY=run
run:
	qemu-system-x86_64 $(VMARGS)

.PHONY=vnc
vnc:
	qemu-system-x86_64 $(VMARGS) -vnc :0

.PHONY=disass
disass:
	$(MAKE) -C $(ISODIR) disass

.PHONY=debug
debug:
	@echo "- - - Si consiglia di non utilizzare nessuna ottimizzazione per far funzionare gdb al meglio - - -"
	qemu-system-x86_64 -s -S $(VMARGS) &
	$(DBG) -q -s $(DBGSYM) -x $(DBGSCR)
	killall qemu-system-x86_64
