ENTRY(read)
SECTIONS
{
    . = 0x7e00;
    .text 0x7e00 :
    {
        *(.text);
    }
    .note.gnu.property : AT (0x7ffe)
    {
        *(.note.gnu.property);
    }
}
