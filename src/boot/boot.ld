OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
ENTRY(_start)

OUTPUT_ARCH(i386)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x07E00, LENGTH = 32K
}

SECTIONS
{
    . = 0x07E00;
    .text : { *(.text) } > RAM
    .data : { *(.data) } > RAM
    .bss : { *(.bss) } > RAM
}
