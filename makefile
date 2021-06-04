DD		= dd
IF  	= init/boot.bin
TARGET	= boot.bin

.PHONY=all
all: $(TARGET)

$(TARGET): $(IF)
	$(DD) seek=0 bs=512 count=1 conv=notrunc if=$^ of=$@

$(IF): init/makefile
	make -C init/

.PHONY=clean
clean:
	make -C init/ clean
	rm *.bin

.PHONY=run
run:
	qemu-system-x86_64 $(TARGET)