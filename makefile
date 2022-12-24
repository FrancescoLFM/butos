DD		= dd

INITDIR	= init
ISODIR	= src

INIT  	= $(INITDIR)/boot.bin
ISO		= $(ISODIR)/butos/butos.bin

BINDIR	= bin
BINFILE	= $(BINDIR)/boot.bin

QEMUDIR = qemu
IMG		= $(QEMUDIR)/vhdd.img
TARGET	= $(IMG)
FORMAT	= raw
SIZE	= 21K
VMARGS	= -device piix3-ide,id=ide -drive id=disk,file=$(IMG),format=raw,if=none -device ide-hd,drive=disk,bus=ide.0 -m 2G

.PHONY=all
all: $(TARGET)

$(TARGET): $(BINFILE)
	@printf "\n[UPDATE] Generazione del disco avviabile\n\n"
	-mkdir $(QEMUDIR)
	qemu-img create -f $(FORMAT) $@ $(SIZE)
	$(DD) if=$^ of=$@ conv=notrunc


$(BINFILE): $(ISO) $(INIT)
	@printf "\n[UPDATE] Generazione dell'eseguibile complessivo\n\n"
	-mkdir bin
	$(DD) seek=0 bs=512 count=1 conv=notrunc if=$(INIT) of=$@
	$(DD) seek=1 bs=512 conv=notrunc if=$(ISO) of=$@

$(INIT):
	@printf "[UPDATE] Compilazione del bootloader in real mode\n\n"
	make -C $(INITDIR)

$(ISO):
	@printf "\n[UPDATE] Compilazione del kernel butos\n\n"
	make -C $(ISODIR)

.PHONY=clean
clean:
	@printf "\n[UPDATE] Pulizia dei file di compilazione nel kernel\n\n"
	make -C $(ISODIR) clean
	@printf "[UPDATE] Pulizia dei file di compilazione nel bootloader\n\n"
	make -C $(INITDIR) clean
	@printf "\n[UPDATE] Rimozione del disco generato\n\n"
	rm $(TARGET)
	rm $(BINFILE)

.PHONY=run
run:
	qemu-system-x86_64 $(VMARGS)

.PHONY=disass
disass:
	make -C $(ISODIR) disass

.PHONY=debug
debug:
	make -C $(ISODIR) debug