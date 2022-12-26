#include <cpu/idt.h>
#include <libs/print.h>
#include <drivers/keyboard.h>

#include <include/asm.h>
#include <include/def.h>
#include <include/trigger.h>

#define RELEASED(SCAN)            ((SCAN) | RELEASED_KEY)

/**
 * Object keyboard
 * - holds the reference of the lookup tables for
 *   converting the scancodes into ascii characters
 * - allows the implementation of different keyboards
 *   for different languages or different setups
 */
struct keyboard {
    uint8_t *std_table;
    uint8_t *shift_table;
    uint8_t *ctrl_table;
    uint8_t *ctrl_alt_table;
    uint8_t *altgr_table;
};

static
uint8_t NORMAL_KEYS_C[] = {
    '\0','\0','1','2','3','4','5','6','7','8','9','0','-','=','\x08','\t','q','w','e','r','t','y','u',
    'i','o','p','[',']','\n','\0','a','s','d','f','g','h','j','k','l',';','\'','`','\0','\\','z','x',
    'c','v','b','n','m',',','.','/','\0','*','\0',' ','\0','\0','\0','\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','7','8','9','-','4','5','6','+','1','2','3','\0','.','\0','\0','\0','\0','\0'
};

static
uint8_t SHIFT_KEYS_C[] = {
    '\0','\0','!','@','#','$','%','^','&','*','(',')','-','+',',','\t', 'Q','W','E','R','T','Y','U','I',
    'O','P','{','}','\0','\0','A','S','D','F','G','H','J','K','L',';', '"','\0','\0','|','Z','X','C','V',
    'B','N','M','<','>','?','\0','*','\0',' ','\0','\0','\0','\0','\0','\0', '\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','-','\0','\0','\0','+','\0','\0','\0','\0','\0','\0'
};

/**
 * Default keyboard
 */
struct keyboard std_keyboard = {
    .std_table = NORMAL_KEYS_C,
    .shift_table = SHIFT_KEYS_C,
    .ctrl_table = NULL,
    .ctrl_alt_table = NULL,
    .altgr_table = NULL,
};

struct keyboard *current_keyboard = &std_keyboard;
trigger_t extended = 0;

int select_keyboard(struct keyboard *keyboard)
{
    if (keyboard->std_table == NULL)
        return 1;
    current_keyboard = keyboard;
    return 0;
}

static
uint16_t keyboard_get_scancode()
{
    uint8_t scan = 0;

    scan = inb(KEYBOARD_DATA_PORT);
    io_delay();
    
    return scan;
}

static
char keyboard_get_ascii(struct keyboard *keyboard, uint8_t scan)
{
    static uint8_t *current_table = NULL;

    switch (scan) {
    case RIGHT_SHIFT_KEY:
    case LEFT_SHIFT_KEY:
        current_table = keyboard->shift_table;
        break;
    
    /* implement other key combinations */
    
    /* put here in fallthrough all the release combinations */
    case (RIGHT_SHIFT_KEY | RELEASED_KEY):
    case (LEFT_SHIFT_KEY | RELEASED_KEY):
        current_table = keyboard->std_table;
        break;
    }

    if (scan & RELEASED_KEY)
        return 0;

    if (current_table == NULL)
        current_table = keyboard->std_table; /* std_table can never be NULL */
    
    return current_table[scan];
}

void keyboard_handler(struct registers_t *regs)
{
    uint8_t scan;
    char chr;
    (void) regs;    /* needed for irq */

    /*static const uint8_t CTRL_SCANCODES[] = {
        LEFT_SHIFT_KEY, RIGHT_SHIFT_KEY, CAPS_LOCK_KEY, LEFT_CONTROL_KEY
    };

    static const uint8_t EXTENDED_CTRL_SCANCODES[] = {
        RIGHT_CONTROL_KEY
    };*/

    scan = keyboard_get_scancode();

    chr = keyboard_get_ascii(current_keyboard, scan);

    if (chr)
        putc(STD_COLOR, chr);
}

/**
 * Sets the irq for the keyboard to work
 */
void keyboard_start()
{
    set_int_handler(IRQ(1), keyboard_handler);
}

/* implement a function to deactivate the keyboard */
