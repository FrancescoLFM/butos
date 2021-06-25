DD		= dd

INITDIR	= init
ISODIR	= src/butos

INIT  	= $(INITDIR)/boot.bin
ISO		= $(ISODIR)/butos.bin

BINDIR	= bin
TARGET	= $(BINDIR)/boot.bin

.PHONY=all
all: $(TARGET)

$(TARGET): $(ISO) $(INIT)
	$(DD) seek=0 bs=512 count=1 conv=notrunc if=$(INIT) of=$@
	$(DD) seek=1 bs=512 conv=notrunc if=$(ISO) of=$@

$(INIT):
	make -C $(INITDIR)

$(ISO):
	make -C $(ISODIR)

.PHONY=clean
clean:
	make -C $(INITDIR) clean
	make -C $(ISODIR) clean
	rm $(TARGET)

.PHONY=run
run:
	qemu-system-x86_64 $(TARGET)

.PHONY=disass
disass:
	make -C $(ISODIR) disass

.PHONY=debug
debug:
	make -C $(ISODIR) debug