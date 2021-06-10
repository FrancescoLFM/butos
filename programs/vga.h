#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_TEXT_START  ((void*)(0xb8000))
#define VGA_TEXT_END    ((void*)(0xb9f40))
#define VGA_COLS        80
#define VGA_WIDTH       VGA_COLS
#define VGA_ROWS        25

struct vga_char {
    char    ascii;
    uint8_t color;
};

struct cursor {
    uint8_t x;
    uint8_t y;
};

void* get_abs_cursor(void* base, uint8_t size, struct cursor crs, uint8_t cols);

void mv_cursor(struct cursor *crs, uint8_t x, uint8_t y);

void mv_vga_pointer(struct cursor crs);

struct vga_char* get_vga_pointer();

struct vga_char* inc_vga_pointer(uint16_t pos);

struct vga_char* dec_vga_pointer(uint16_t pos);

struct vga_char* vga_allign_left();

struct vga_char* vga_newline();

void print_pm(int color, char* string);

/* Cursor */

// default (13, 14)
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);

void disable_cursor();

void update_cursor(struct cursor crs);

void inc_cursor(uint8_t pos);

void dec_cursor(uint8_t pos);

#endif /* VGA_H */