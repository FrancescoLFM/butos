# Shared toolchain configuration, included by every makefile.
# On macOS the host gcc is clang and cannot produce i386 ELF binaries,
# so the i686-elf cross toolchain (brew install i686-elf-gcc) is used.
# Override with e.g. `make CROSS=i686-elf-` on Linux.

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
CROSS ?= i686-elf-
else
CROSS ?=
endif

CC      = $(CROSS)gcc
AS      = $(CROSS)as
LD      = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
STRIP   = $(CROSS)strip

define color_text
	@printf "\033[$1m%s\033[0m\n" $2
endef
