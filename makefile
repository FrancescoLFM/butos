DD		= dd
IF  	= programs/read.bin
TARGET	= boot

.PHONY=all
all: $(TARGET)
$(TARGET): init/makefile programs/makefile
	make -C init/
	make -C programs/
	$(DD) seek=1 bs=512 count=1 if=$(IF) of=$(TARGET).bin

.PHONY=run
run:
	qemu-system-x86_64 $(TARGET).bin
