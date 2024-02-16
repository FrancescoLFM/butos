OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
ENTRY(_start)

OUTPUT_ARCH(i386)

MEMORY
{
    RAM (rwx) : ORIGIN = 0xf000, LENGTH = 100K
}

SECTIONS
{
    . = 0xf000;
    .text : { *(.text) } > RAM
    .data : { *(.data) } > RAM
    .bss : { *(.bss) } > RAM
}
