#include <cpu/idt.h>
#include <libs/print.h>
#include <libs/string.h>
#include <drivers/keyboard.h>
#include <include/asm.h>
#include <include/trigger.h>
#include <libs/alloc.h>

#define EXTENDED_SCANCODE         0xE0
#define RELEASED(SCAN)            ((SCAN) | RELEASED_KEY)
#define UNRELEASED(SCAN)          ((SCAN) & ~RELEASED_KEY)

#define IN_ARRAY(ELEM, ARRAY)  (memchr((ARRAY), (ELEM), sizeof(ARRAY)) != NULL)

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
    uint8_t *altgr_table;

    uint8_t *selected;
    flag_t caps_lock;
};

uint8_t STANDARD_KEYS[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9','0', '\'', 'i', '\x08',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'e', '+', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'i', 'k', 'l', 'o', 'a', '\\', 0, 'u', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '<', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

uint8_t SHIFT_KEYS[] = {
    0, 0, '!', '"', 0, '$', '%', '&', '/', '(', ')', '=', '?', '^', '\x08',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 'e', '*', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'I', 'K', 'L', 'o', 'a', '|', 0, 'u', 'Z',
    'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '>', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

uint8_t ALTGR_KEYS[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '{', '[', ']', '}', '`', '~', '\x08',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'i', 'k', 'l', '@', '#', '\\', 0, 'u', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '<', 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static queue_t input_buffer_;
queue_t *input_buffer = &input_buffer_;

/**
 * Default keyboard
 */
struct keyboard std_keyboard = {
    .std_table = STANDARD_KEYS,
    .shift_table = SHIFT_KEYS,
    .altgr_table = ALTGR_KEYS,

    .selected = STANDARD_KEYS,
    .caps_lock = 0
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

/**
 * Selects the current table of characters from
 * the ones in the keyboard object
 */
static
void keyboard_change_table(struct keyboard *keyboard, uint8_t scan, int ext)
{
    if (!ext) switch (scan) {
        case RIGHT_SHIFT_KEY:
        case LEFT_SHIFT_KEY:
            keyboard->selected = keyboard->shift_table;
            break;
        
        case CAPS_LOCK_KEY:
            keyboard->caps_lock ^= 1;
            break;
        
        /* altri tasti  standard */
    } else switch (scan) {
        case ALTGR_KEY:
            keyboard->selected = keyboard->altgr_table;
            break;
        
        /* altri tasti estesi */
    }

    if (scan & RELEASED_KEY)
        keyboard->selected = keyboard->caps_lock ? 
            keyboard->shift_table : keyboard->std_table;
}

static
uint8_t keyboard_read_character(struct keyboard *keyboard, uint8_t scan)
{
    if (scan & RELEASED_KEY)
        return 0;
    
    if (keyboard->selected)
        return keyboard->selected[scan];
    return keyboard->std_table[scan];
}

void keyboard_handler(struct registers_t *regs)
{
    uint8_t scan;
    char chr;
    int ext;

    (void) regs;    /* needed for irq */

    static const uint8_t CTRL_SCANCODES[] = {
        LEFT_SHIFT_KEY, RIGHT_SHIFT_KEY, CAPS_LOCK_KEY, LEFT_CONTROL_KEY,
        ALT_KEY
    };

    static const uint8_t EXTENDED_CTRL_SCANCODES[] = {
        RIGHT_CONTROL_KEY, MOD_KEY, ALTGR_KEY, UP_ARROW_KEY, RIGHT_ARROW_KEY,
        LEFT_ARROW_KEY, DOWN_ARROW_KEY
    };

    ext = trigger_is_set(&extended);

    scan = keyboard_get_scancode();
    if (scan == EXTENDED_SCANCODE) {  /* extended */

        trigger_set(&extended);

    } else if ((!ext && IN_ARRAY(UNRELEASED(scan), CTRL_SCANCODES)) ||
        (ext && IN_ARRAY(UNRELEASED(scan), EXTENDED_CTRL_SCANCODES))) {
        /* special keys */
    
        keyboard_change_table(current_keyboard, scan, ext);

    } else {  /* normal keys */

        chr = keyboard_read_character(current_keyboard, scan);
        if (chr) {
            putc(STD_COLOR, chr);
            queue_enqueue(input_buffer, chr);
        }
        
    }
}

/**
 * Sets the irq for the keyboard to work
 */
void keyboard_start(size_t input_buffer_size)
{
    queue_init(input_buffer, input_buffer_size);
    set_int_handler(IRQ(1), keyboard_handler);
}

/* implement a function to deactivate the keyboard */
