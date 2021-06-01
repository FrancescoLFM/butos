NAME	= boot

AS		= as
LD		= ld
LDSCR	= $(NAME).ld

TARGET	= ../$(NAME).bin
OBJ		= init.o write.o diskout.o ../bioslib.o
ENTRY	= init

.PHONY=all
all: $(TARGET)

%.o: %.s
	$(AS) $< -o $@

$(TARGET): $(OBJ)
	$(LD) -T $(LDSCR) $^ -o $(NAME).out
	strip --remove-section=.note.gnu.property $(NAME).out
	objcopy -O binary -S $(NAME).out $@
	rm $(NAME).out

.PHONY=clean
clean:
	rm *.o $(TARGET)