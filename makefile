DD		= dd
IF  	= 
TARGET	= boot.bin

.PHONY=all
all: $(TARGET)
$(TARGET): $(IF)
	make -C init/
	$(DD) seek=1 bs=512 count=1 conv=notrunc if=$^ of=$@

.PHONY=clean
clean:
	rm *.bin

.PHONY=run
run:
	qemu-system-x86_64 $(TARGET)