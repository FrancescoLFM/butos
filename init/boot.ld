ENTRY(init);
SECTIONS
{
    . = 0x7c00;
    .text 0x7c00 :
    {
        *(.text);
    }
    .sig 0x7dfe :
    {
        SHORT(0xaa55);
    }
    .note.gnu.property : AT (ADDR(.sig) + SIZEOF(.sig))
    {
        *(.note.gnu.property);
    }
}