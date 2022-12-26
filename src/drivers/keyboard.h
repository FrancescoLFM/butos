#ifndef DRIVERS_KB_H
#define DRIVERS_KB_H

#include <include/def.h>

/**
 * Documented at: https://wiki.osdev.org/%228042%22_PS/2_Controller
 */

#define KEYBOARD_DATA_PORT          0x60
#define KEYBOARD_STATUS_REGISTER    0x64
#   define OUTPUT_BUFFER_STATUS     BIT(0)
#   define INPUT_BUFFER_STATUS      BIT(1)
#   define SYSTEM_FLAG              BIT(2)
#   define COMMAND_DATA             BIT(3)
#   define TIME_OUT_ERROR           BIT(6)
#   define PARITY_ERROR             BIT(7)
#define KEYBOARD_COMMAND_REGISTER   0x64

#define RELEASED_KEY        BIT(7)

/* standard ctrl scancodes */
#define LEFT_SHIFT_KEY      0x2A
#define RIGHT_SHIFT_KEY     0x36
#define CAPS_LOCK_KEY       0x3A
#define LEFT_CONTROL_KEY    0x1D

/* extended ctrl scancodes */

#define RIGHT_CONTROL_KEY   0x1D

void keyboard_start();

#endif