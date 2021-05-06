AS=as
LD=ld
LDARGS=-Tboot.ld --oformat binary

TARGET=boot.bin
OBJ=init.o
ENTRY=init

.PHONY=all
all: clean $(TARGET)

%.o: %.s
	$(AS) $< -o $@

$(TARGET): $(OBJ)
	$(LD) $(LDARGS) -e $(ENTRY) $^ -o $@

.PHONY=clean
clean:
	rm *.o $(TARGET)

.PHONY=test
test:
	qemu-system-x86_64 $(TARGET)