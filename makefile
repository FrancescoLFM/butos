DD		= dd

INITDIR	= init
ISODIR	= src/butos

INIT  	= $(INITDIR)/boot.bin
ISO		= $(ISODIR)/butos.bin

BINDIR	= bin
TARGET	= $(BINDIR)/boot.bin

QEMUDIR = qemu
IMG		= $(QEMUDIR)/vhdd.img
FORMAT	= raw
SIZE	= 100M
VMARGS	= -device piix3-ide,id=ide -drive id=disk,file=$(IMG),format=raw,if=none -device ide-hd,drive=disk,bus=ide.0

.PHONY=all
all: $(TARGET)

$(TARGET): $(ISO) $(INIT)
	$(DD) seek=0 bs=512 count=1 conv=notrunc if=$(INIT) of=$@
	$(DD) seek=1 bs=512 conv=notrunc if=$(ISO) of=$@
	qemu-img create -f $(FORMAT) $(IMG) $(SIZE)
	$(DD) if=bin/boot.bin of=$(IMG)

$(INIT):
	make -C $(INITDIR)

$(ISO):
	make -C $(ISODIR)

.PHONY=clean
clean:
	make -C $(INITDIR) clean
	make -C $(ISODIR) clean
	rm $(TARGET)
	rm $(IMG)

.PHONY=run
run:
	qemu-system-x86_64 $(VMARGS)

.PHONY=disass
disass:
	make -C $(ISODIR) disass

.PHONY=debug
debug:
	make -C $(ISODIR) debug