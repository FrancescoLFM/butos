OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
ENTRY(main)

OUTPUT_ARCH(i386)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x0100000, LENGTH = 1M
}

SECTIONS
{
    . = 0x0100000;
    .text : { *(.text) } > RAM
    .data : { *(.data) } > RAM
    .bss : { *(.bss) } > RAM
}
