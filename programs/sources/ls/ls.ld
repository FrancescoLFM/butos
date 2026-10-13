OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
ENTRY(main)

OUTPUT_ARCH(i386)

/* Same layout as datatest */
SECTIONS
{
    . = 0x01000000;
    .text : { *(.text) *(.rodata*) }
    . = ALIGN(0x1000);
    .data : { *(.data) }
    .bss : { *(.bss) *(COMMON) }
}
