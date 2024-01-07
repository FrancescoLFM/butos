MEMORY
{
    BOOTSTRAP (rx) : ORIGIN = 0x0600, LENGTH = 0x1B8
    MBR (rx) : ORIGIN = 0x7B8, LENGTH = 0x48
}

SECTIONS
{
    .text : {
        *(.text);
    } > BOOTSTRAP 

    .data : {
        *(.mbr);
    } > MBR
}
