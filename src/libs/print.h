#ifndef LIBS_PRINT_H
#define LIBS_PRINT_H

#include <stdint.h>
#include <include/colors.h>

int print_pm(uint8_t color, char* string);
int puts(char* string);
int putc(uint8_t color, char c);
int printk(uint8_t color, char *str, ...);

/* Cursor */

struct cursor {
    uint8_t x;
    uint8_t y;
};

// default (13, 14)
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void update_cursor(struct cursor crs);
void inc_cursor(int pos);

#endif