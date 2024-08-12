DD		= dd

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
MOUNTDIR= $(QEMUDIR)/mount/
TARGET	= $(IMG)
SIZE	= 200K
FORMAT  = raw
VMARGS	= -device piix3-ide,id=ide -drive id=disk,file=$(IMG),format=$(FORMAT),if=none -device ide-hd,drive=disk,bus=ide.0 -m 2G

DBG     = gdb
DBGSYM  = src/butos
DBGSCR  = scripts/butos.gdb

PROGDIR = programs
PROGBIN = $(PROGDIR)/bin/*

define color_text
	@echo -e "\033[$1m$2\033[0m"
endef

.PHONY=all
all:
	$(call color_text,91,"[MAKE] Compilazione del bootloader butos")
	make -C $(BOOTDIR)
	$(call color_text,91,"[MAKE] Compilazione del kernel butos")
	make -C $(ISODIR)
	$(call color_text,91,"[MAKE] Compilazione del bootloader in real mode")
	make -C $(INITDIR)
	make $(TARGET)
	$(call color_text,91,"[MAKE] Creazione della partizione FAT32")
	rm -f $(PART)
	mkfs.fat -F 32 --mbr=y -C $(PART) 1000
	$(call color_text,91,"[MAKE] Compilazione degli eseguibili di base")
	make -C $(PROGDIR)
	$(call color_text,91,"[MAKE] Montaggio della partizione")
	sudo mount -v $(PART) $(MOUNTDIR)
	for i in $(PROGBIN) ; do \
		sudo cp $$i $(MOUNTDIR); \
	done
	sudo umount -v $(MOUNTDIR)
	dd if=$(PART) of=$(IMG) seek=203 

$(TARGET): $(BINFILE)
	$(call color_text,91,"[MAKE] Generazione del disco avviabile")
	-mkdir $(QEMUDIR)
	qemu-img create -f $(FORMAT) $@ $(SIZE)
	$(DD) if=$^ of=$@ conv=notrunc


$(BINFILE): $(ISO) $(INIT)
	$(call color_text,91,"[MAKE] Generazione dell eseguibile complessivo")
	-mkdir bin
	$(DD) seek=0 bs=512 count=2 conv=notrunc if=$(INIT) of=$@
	$(DD) seek=2 bs=512 conv=notrunc if=$(BOOTFILE) of=$@
	$(DD) seek=66 bs=512 conv=notrunc if=$(ISO) of=$@

.PHONY=silent
silent:
	@make -s 2>/dev/null

.PHONY=clean
clean:
	$(call color_text,91,"[MAKE] Pulizia dei file di compilazione nel kernel bootloader")
	make -C $(BOOTDIR) clean
	$(call color_text,91,"[MAKE] Pulizia dei file di compilazione nel kernel")
	make -C $(ISODIR) clean
	$(call color_text,91,"[MAKE] Pulizia dei file di compilazione nel bootloader")
	make -C $(INITDIR) clean
	$(call color_text,91,"[MAKE] Rimozione del disco generato")
	rm $(TARGET)
	rm $(BINFILE)

.PHONY=run
run:
	qemu-system-x86_64 $(VMARGS)

.PHONY=vnc
vnc:
	qemu-system-x86_64 $(VMARGS) -vnc :0

.PHONY=disass
disass:
	make -C $(ISODIR) disass

.PHONY=debug
debug:
	@echo "- - - Si consiglia di non utilizzare nessuna ottimizzazione per far funzionare gdb al meglio - - -"
	qemu-system-x86_64 -s -S $(VMARGS) &
	$(DBG) -q -s $(DBGSYM) -x $(DBGSCR)
	killall qemu-system-x86_64
