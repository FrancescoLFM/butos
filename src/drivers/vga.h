#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <include/colors.h>

#define VGA_TEXT_START  ((struct vga_char *)(0xb8000))
#define VGA_TEXT_END    ((struct vga_char *)(0xb9f40))
#define VGA_COLS        80
#define VGA_WIDTH       VGA_COLS
#define VGA_ROWS        25

int vga_inc_pointer(int pos);
void vga_write(uint8_t color, char c);
int vga_newline();
int vga_allign_left();
int vga_tab();

#endif