MEMORY
{
    BOOTLOADER (rx) : ORIGIN = 0x7c00, LENGTH = 0x200
}

SECTIONS
{
    .text : {
        *(.text);
    } > BOOTLOADER

    .data : {
        *(.data);
    } > BOOTLOADER
}
