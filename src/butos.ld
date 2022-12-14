OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)

MEMORY
{
    LANFREDI (rx) : ORIGIN = 0xf000, LENGTH = 20K
}

SECTIONS
{
    .text : { *(.text) } > LANFREDI
    .data : { *(.data) } > LANFREDI
    .bss : { *(.bss) } > LANFREDI
}