#ifndef KB_H
#define KB_H

#define ARR_SIZE(arr)   ((sizeof(arr)) / (sizeof(arr[0])))

#define SCANCODES    ARR_SIZE(NORMAL_KEYS_C)

void init_kb();

#endif