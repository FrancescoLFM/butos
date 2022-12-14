#ifndef LIBS_PRINT_H
#define LIBS_PRINT_H

#include <stdint.h>
#include <include/colors.h>

int print_pm(uint8_t color, char* string);
int puts(char* string);
void putc(uint8_t color, char c);
int printk(uint8_t color, char *str, ...);

#endif