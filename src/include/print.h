#ifndef VGA_H
#define VGA_H

#include <include/def.h>

#define VGA_TEXT_START  ((void*)(0xb8000))
#define VGA_TEXT_END    ((void*)(0xb9f40))
#define VGA_COLS        80
#define VGA_WIDTH       VGA_COLS
#define VGA_ROWS        25

#define BACKGROUND(col) ((col) << 4)
#define FOREGROUND(col) (col)
#define COLOR(fg, bg)   (FOREGROUND(fg) | BACKGROUND(bg))
#define RED             0b100
#define GREEN           0b010
#define BLUE            0b001
#define WHITE           0b111
#define BLACK           0b000

#define STD_COLOR  0b00000111

struct vga_char {
    char    ascii;
    uint8_t color;
};

struct cursor {
    uint8_t x;
    uint8_t y;
};

int print_pm(uint8_t color, char* string);

int puts(char* string);

int putc(uint8_t color, char c);

int printk(uint8_t color, char *str, ...);

/* Cursor */

// default (13, 14)
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);

void disable_cursor();

void update_cursor(struct cursor crs);

void inc_cursor(uint8_t pos);

void dec_cursor(uint8_t pos);

#endif /* VGA_H */