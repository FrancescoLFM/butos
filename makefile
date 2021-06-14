DD		= dd
IF  	= init/boot.bin programs/lanfredi.bin
TARGET	= boot.bin

.PHONY=all
all: $(TARGET)

$(TARGET): $(IF)
	$(DD) seek=0 bs=512 count=1 conv=notrunc if=init/boot.bin of=$@
	$(DD) seek=1 bs=512 conv=notrunc if=programs/lanfredi.bin of=$@

$(IF): init/makefile programs/makefile clibs/makefile
	make -C init/
	make -C clibs/
	make -C programs/

.PHONY=clean
clean:
	make -C init/ clean
	make -C clibs/ clean
	make -C programs/ clean
	rm *.bin

.PHONY=run
run:
	qemu-system-x86_64 $(TARGET)