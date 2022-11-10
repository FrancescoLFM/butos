#ifndef KB_H
#define KB_H

#include <include/array.h>

#define ARR_SIZE(arr)   ((sizeof(arr)) / (sizeof(arr[0])))

#define MAX_SCANCODES   ARR_SIZE(NORMAL_KEYS_C)

#define RELEASED_KEY    (1 << 7)
#define SHIFT_KEY       0x2a

void init_kb();

#endif