#ifndef DRIVERS_KB_H
#define DRIVERS_KB_H

#include <libs/array.h>
#include <include/def.h>

#define MAX_SCANCODES   ARR_SIZE(NORMAL_KEYS_C)

#define RELEASED_KEY    (1 << 7)
#define SHIFT_KEY       0x2a

void init_kb();

#endif