#include <include/asm.h>
#include <include/idt.h>
#include <include/print.h>
#include <include/kb.h>

const uint8_t NORMAL_KEYS_C[] = {0, 0, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 0, 0, 113, 119, 
                                 101, 114, 116, 121, 117, 105, 111, 112, 91, 93, 0, 0, 97, 115, 100, 102, 103, 104, 106, 107, 108, 59, 
                                 39, 96, 0, 92, 122, 120, 99, 118, 98, 110, 109, 44, 46, 47, 0, 42, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                                 0, 0, 0, 0, 55, 56, 57, 45, 52, 53, 54, 43, 49, 50, 51, 0, 46, 0, 0, 0, 0, 0};

uint8_t get_scancode()
{
    uint8_t c = inb(0x60);

    io_delay();
    return c;
}

char get_ascii(uint8_t scancode)
{
    if (scancode >= SCANCODES) 
        return -1;

    return NORMAL_KEYS_C[scancode];
}

void kb_handler(struct registers_t *regs)
{
    uint8_t scan = get_scancode();
    char c;
    
    if (scan & (1 << 7))
        return;
    
    c = get_ascii(scan);
    if(c)
        putc(STD_COLOR, c);
}

void init_kb()
{
    set_int_handler(IRQ(1), kb_handler);
}