#include <include/asm.h>
#include <cpu/idt.h>
#include <libs/print.h>
#include <drivers/keyboard.h>

static const
uint8_t NORMAL_KEYS_C[] = {
    '\0','\0','1','2','3','4','5','6','7','8','9','0','-','=','\x08','\t','q','w','e','r','t','y','u',
    'i','o','p','[',']','\n','\0','a','s','d','f','g','h','j','k','l',';','\'','`','\0','\\','z','x',
    'c','v','b','n','m',',','.','/','\0','*','\0',' ','\0','\0','\0','\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','7','8','9','-','4','5','6','+','1','2','3','\0','.','\0','\0','\0','\0','\0'
};

static const
uint8_t SHIFT_KEYS_C[] = {
    '\0','\0','!','@','#','$','%','^','&','*','(',')','-','+',',','\t', 'Q','W','E','R','T','Y','U','I',
    'O','P','{','}','\0','\0','A','S','D','F','G','H','J','K','L',';', '"','\0','\0','|','Z','X','C','V',
    'B','N','M','<','>','?','\0','*','\0',' ','\0','\0','\0','\0','\0','\0', '\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','-','\0','\0','\0','+','\0','\0','\0','\0','\0','\0'
};

static
uint8_t const *table = NORMAL_KEYS_C;

uint8_t get_scancode()
{
    uint8_t c = inb(0x60);

    io_delay();
    return c;
}

char get_ascii(uint8_t scancode)
{
    switch (scancode) {
    case SHIFT_KEY:
        table = SHIFT_KEYS_C;
        break;
    
    case (SHIFT_KEY | RELEASED_KEY):
        table = NORMAL_KEYS_C;
        break;
    }

    if (scancode & RELEASED_KEY)
        return 0;
    
    return table[scancode];
}

void kb_handler(struct registers_t *regs)
{
    uint8_t scan;
    char c;
    (void) regs;

    scan = get_scancode();

    c = get_ascii(scan);

    if (c)
        putc(STD_COLOR, c);
}

void init_kb()
{
    set_int_handler(IRQ(1), kb_handler);
}