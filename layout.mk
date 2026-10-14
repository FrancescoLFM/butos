# Disk layout (in 512-byte sectors), computed from the size of the built
# binaries: growing stage 2 or the kernel just shifts everything after it.
# Included by every makefile that bakes or uses these offsets; values are
# exported to the assembly with --defsym (see LAYOUT_DEFS).
#
#   0             MBR (init/mbr)
#   1             real-mode bootloader (init)
#   STAGE2_LBA    stage 2 (src/boot/boot.bin)
#   KERNEL_LBA    kernel (src/butos.bin)
#   FAT_LBA       FAT32 partition

LAYOUT_ROOT := $(dir $(lastword $(MAKEFILE_LIST)))

sectors_of = $(shell f=$(1); [ -f "$$f" ] && echo $$(( ($$(wc -c < "$$f") + 511) / 512 )) || echo 0)

STAGE2_BIN      = $(LAYOUT_ROOT)src/boot/boot.bin
KERNEL_BIN      = $(LAYOUT_ROOT)src/butos.bin

STAGE2_LBA      = 2
STAGE2_SECTORS  = $(call sectors_of,$(STAGE2_BIN))
KERNEL_LBA      = $(shell echo $$(( $(STAGE2_LBA) + $(STAGE2_SECTORS) )))
KERNEL_SECTORS  = $(call sectors_of,$(KERNEL_BIN))
FAT_LBA         = $(shell echo $$(( $(KERNEL_LBA) + $(KERNEL_SECTORS) )))

# Real-mode load windows (addresses in init/include/macro.s): stage 2 goes
# at 0x7E00 below the kernel buffer at 0xF000, the kernel from 0xF000 up to
# the EBDA at 0x9FC00.
STAGE2_MAX_SECTORS = 57
KERNEL_MAX_SECTORS = 1158

LAYOUT_DEFS     = --defsym STAGE2_LBA=$(STAGE2_LBA) \
                  --defsym STAGE2_SECTORS=$(STAGE2_SECTORS) \
                  --defsym KERNEL_LBA=$(KERNEL_LBA) \
                  --defsym SEC=$(KERNEL_SECTORS) \
                  --defsym FAT_LBA=$(FAT_LBA)
