OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
ENTRY(main)

OUTPUT_ARCH(i386)

MEMORY
{
    RAM (rwx) : ORIGIN = 0xC0000000, LENGTH = 10M
}

SECTIONS
{
    . = 0xC0000000;
    /* Stage 2 jumps to 0xC0000000, so main has to come first */
    .text : {
        *(.text.entry)
        *(.text)
    } > RAM
    .data : { *(.data) } > RAM
    .bss : {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    } > RAM
}
