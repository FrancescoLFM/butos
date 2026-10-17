#include <cpu/idt.h>
#include <libs/string.h>
#include <drivers/keyboard.h>
#include <include/asm.h>
#include <include/trigger.h>
#include <libs/alloc.h>
#include <drivers/keymaps.h>

#define EXTENDED_SCANCODE         0xE0
#define RELEASED(SCAN)            ((SCAN) | RELEASED_KEY)
#define UNRELEASED(SCAN)          ((SCAN) & ~RELEASED_KEY)

#define IN_ARRAY(ELEM, ARRAY)  (memchr((ARRAY), (ELEM), sizeof(ARRAY)) != NULL)

#ifndef KEYMAP_DEFAULT
#define KEYMAP_DEFAULT  "it"
#endif

static queue_t input_buffer_;
queue_t *input_buffer = &input_buffer_;

static const struct keymap *current_keymap = &keymaps[0];

/* Modifier state: the table a key reads from depends on what is held right now */
static int shift_held = 0;      /* bit 0 left shift, bit 1 right shift */
static int altgr_held = 0;
static int caps_lock = 0;
trigger_t extended = 0;
/* Either control key is held: letters become control characters (Ctrl+D is 0x04) */
static int control_held = 0;

int keyboard_set_layout(const char *name)
{
    for (size_t i = 0; i < ARR_SIZE(keymaps); i++) {
        if (strcmp(keymaps[i].name, name) == 0) {
            current_keymap = &keymaps[i];
            return 0;
        }
    }

    return 1;
}

const char *keyboard_layout(void)
{
    return current_keymap->name;
}

static
uint16_t keyboard_get_scancode()
{
    uint8_t scan = 0;

    scan = inb(KEYBOARD_DATA_PORT);
    io_delay();
    
    return scan;
}

/* Tracks the modifiers, press and release */
static
void keyboard_modifier(uint8_t scan, int ext)
{
    int pressed = !(scan & RELEASED_KEY);
    int bit;

    scan = UNRELEASED(scan);
    if (!ext) switch (scan) {
        case LEFT_SHIFT_KEY:
        case RIGHT_SHIFT_KEY:
            bit = scan == LEFT_SHIFT_KEY ? 1 : 2;
            shift_held = pressed ? (shift_held | bit) : (shift_held & ~bit);
            break;

        case CAPS_LOCK_KEY:
            if (pressed)
                caps_lock ^= 1;
            break;

        case LEFT_CONTROL_KEY:
            control_held = pressed;
            break;
    } else switch (scan) {
        case ALTGR_KEY:
            altgr_held = pressed;
            break;

        case RIGHT_CONTROL_KEY:
            control_held = pressed;
            break;
    }
}

static
uint8_t keyboard_read_character(uint8_t scan)
{
    if (scan & RELEASED_KEY)
        return 0;
    if (altgr_held)
        return current_keymap->altgr_table[scan];
    if (shift_held || caps_lock)
        return current_keymap->shift_table[scan];

    return current_keymap->std_table[scan];
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
        keyboard_modifier(scan, ext);

    } else {  /* normal keys */

        chr = keyboard_read_character(scan);
        if (control_held && ((chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z')))
            chr &= 0x1F;
        if (chr) {
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
    keyboard_set_layout(KEYMAP_DEFAULT);
    set_int_handler(IRQ(1), keyboard_handler);
}

/* implement a function to deactivate the keyboard */
