SHELL		= /bin/bash

CC			= gcc
CFLAGS		= -ffreestanding -m32 -std=gnu99 -O2 -Wall -Wextra -Werror -fno-pie -fno-stack-protector
LD			= ld

PROGDIR		= ..
LIBDIR		= $(PROGDIR)/libs
SCRIPTDIR 	= ../$(PROGDIR)/scripts
CONFDIR		= ../$(PROGDIR)/conf

CFLAGS 	   += -I$(PROGDIR)/

OBJ			= prova.o $(LIBDIR)/print.o $(LIBDIR)/string.o
NAME		= butos
TARGET		= $(NAME).bin
LDSCRIPT	= $(NAME).ld
CONFIGURE	= $(SCRIPTDIR)/configure.sh

.PHONY=all
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBDIR)/%.o: $(LIBDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME).out: $(OBJ)
	$(LD) $^ -o $@ -T $(LDSCRIPT) --ignore-unresolved-symbol _GLOBAL_OFFSET_TABLE_ 

$(TARGET): $(NAME).out
	objcopy	-O binary -S $^ $@
	$(CONFIGURE) $@ $(CONFDIR)
	rm $^

.PHONY=disass
disass:
	make $(NAME).out
	objdump -mi386 -d $(NAME).out
	rm $(NAME).out

.PHONY=clean
clean:
	rm *.o $(LIBDIR)/*.o $(TARGET)