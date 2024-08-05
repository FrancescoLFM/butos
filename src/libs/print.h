#ifndef LIBS_PRINT_H
#define LIBS_PRINT_H

#include <stdint.h>
#include <include/colors.h>
#include <stdarg.h>

int putc_c(uint8_t color, int c);
int puts_c(uint8_t color, const char* string);
int printk_c(uint8_t color, char *str, ...);

int putc(int c);
int puts(const char* string);
int printk(char *str, ...);
int vprintk_c(uint8_t color, char *str, va_list list);

#endif
