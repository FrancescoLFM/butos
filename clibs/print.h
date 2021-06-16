#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include "def.h"

#define VGA_TEXT_START  ((void*)(0xb8000))
#define VGA_TEXT_END    ((void*)(0xb9f40))
#define VGA_COLS        80
#define VGA_WIDTH       VGA_COLS
#define VGA_ROWS        25

#define WHITE_ON_BLACK  0b00000111

struct vga_char {
    char    ascii;
    uint8_t color;
};

struct cursor {
    uint8_t x;
    uint8_t y;
};

int print_pm(int color, char* string);

/* Cursor */

// default (13, 14)
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);

void disable_cursor();

void update_cursor(struct cursor crs);

void inc_cursor(uint8_t pos);

void dec_cursor(uint8_t pos);

#endif /* VGA_H */