include ../../toolchain.mk

# Libraries shared with the kernel, linked by programs as libbutos.a
# (LDLIBS = ../../libs/libbutos.a in a program makefile)

FLAGS = -Wall -Wextra -Werror
FLAGS += -fno-pie -m32
FLAGS += -g
FLAGS += -I..

CFLAGS = $(FLAGS)
CFLAGS += -ffreestanding -nostdlib -fno-stack-protector -std=c99 -mgeneral-regs-only

SOURCES_C = $(wildcard *.c)
OBJECTS = $(SOURCES_C:.c=.o)
LIBRARY = libbutos.a

.PHONY=all
all: $(LIBRARY)

# string.c includes the kernel source
string.o: ../../src/libs/string.c ../../src/libs/string.h

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@ $^

.PHONY=clean
clean:
	-rm -f $(OBJECTS)
	-rm -f $(LIBRARY)
