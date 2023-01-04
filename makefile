DD		= dd

INITDIR	= init
ISODIR	= src

INIT  	= $(INITDIR)/boot.bin
ISO		= $(ISODIR)/butos.bin

BINDIR	= bin
BINFILE	= $(BINDIR)/boot.bin

QEMUDIR = qemu
IMG		= $(QEMUDIR)/vhdd.img
TARGET	= $(IMG)
FORMAT	= raw
SIZE	= 30K
VMARGS	= -device piix3-ide,id=ide -drive id=disk,file=$(IMG),format=raw,if=none -device ide-hd,drive=disk,bus=ide.0 -m 2G

define color_text
	@echo -e "\033[$1m$2\033[0m"
endef

.PHONY=all
all:	
	$(call color_text,91,"[MAKE] Compilazione del kernel butos")
	make -C $(ISODIR)
	$(call color_text,91,"[MAKE] Compilazione del bootloader in real mode")
	make -C $(INITDIR)
	make $(TARGET)

$(TARGET): $(BINFILE)
	$(call color_text,91,"[MAKE] Generazione del disco avviabile")
	-mkdir $(QEMUDIR)
	qemu-img create -f $(FORMAT) $@ $(SIZE)
	$(DD) if=$^ of=$@ conv=notrunc


$(BINFILE): $(ISO) $(INIT)
	$(call color_text,91,"[MAKE] Generazione dell eseguibile complessivo")
	-mkdir bin
	$(DD) seek=0 bs=512 count=1 conv=notrunc if=$(INIT) of=$@
	$(DD) seek=1 bs=512 conv=notrunc if=$(ISO) of=$@

.PHONY=silent
silent:
	@make -s 2>/dev/null

.PHONY=clean
clean:
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

.PHONY=disass
disass:
	make -C $(ISODIR) disass
