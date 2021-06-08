MEMORY
{
    BOOTLOADER (rx) : ORIGIN = 0x7c00, LENGTH = 1K
}

SECTIONS
{
    .text :
    {
        *(.text);
    } > BOOTLOADER
}