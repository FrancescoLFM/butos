#ifndef LIBS_PRINT_H
#define LIBS_PRINT_H

#include <stdint.h>
#include <include/def.h>
#include <include/colors.h>
#include <stdarg.h>

int putc_c(uint8_t color, int c);
int puts_c(uint8_t color, const char* string);
int printk_c(uint8_t color, char *str, ...);

int putc(int c);
int puts(const char* string);
int printk(char *str, ...);
int vprintk_c(uint8_t color, char *str, va_list list);
/* Character sink of vprintk_out, ctx is passed through */
typedef void (*print_out_t)(void *ctx, char c);
/* Formats like printk sending every character to out, returns the conversions done */
int vprintk_out(print_out_t out, void *ctx, char *str, va_list list);
/* Writes num in base (2 to 16) NUL-terminated, returns the digits or -1 if size is too small */
int base_convert(char *buffer, size_t size, uint32_t num, uint32_t base);

#endif
