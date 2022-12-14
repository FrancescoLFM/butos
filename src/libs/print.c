#include <libs/print.h>
#include <drivers/vga.h>
#include <include/asm.h>
#include <stdarg.h>

static int putc_internal(uint8_t color, char c)
{
    int disp = 0;

    switch (c) {
    case '\n':
        if (vga_newline())
            disp += VGA_COLS;
        __fallthrough;
    
    case '\r':
        return disp - vga_allign_left();

    case '\b':
        if (!vga_inc_pointer(-1))
            return 0;
        vga_write(color, ' ');
        return -1;
    
    case '\t':
        return vga_tab();

    case 0:
        return 0;
    
    default:
        vga_write(color, c);
        return vga_inc_pointer(1);
    }

    return 0;
}

int putc(uint8_t color, char c)
{
    int relpos;

    if (!c)
        return 0;
    
    relpos = putc_internal(color, c);
    inc_cursor(relpos);

    return relpos;
}

int print_pm(uint8_t color, char* string)
{
    int count = 0;
    int relpos = 0;

    while (*string) {
        relpos += putc_internal(color, *string++);
        count++;
    }

    inc_cursor(relpos);

    return count;
}

int puts(char* str)
{
    return print_pm(STD_COLOR, str);
}

static char* digits = "0123456789abcdef";
#define MIN_BASE    2
#define MAX_BASE    16

static int base_convert_r(char *buffer, size_t size, uint32_t num, uint32_t base)
{
    int depth;

    if (num <= 0)
        return 0;

    depth = base_convert_r(buffer, size, num / base, base);
    if (depth < 0 || (size_t)depth >= size)
        return -1;
    
    buffer[depth] = digits[num % base];
    
    return depth + 1;
}

int base_convert(char *buffer, size_t size, uint32_t num, uint32_t base)
{
    int length;

    if (!IN_BOUNDS_EQ(MIN_BASE, base, MAX_BASE))
        return -1;
    
    if (size < 2)
        return -1;
    
    if (num == 0) {
        buffer[0] = digits[0];
        buffer[1] = '\0';
        return 1;
    }

    length = base_convert_r(buffer, size-1, num, base);
    if (length < 0)
        return -1;
    
    buffer[length] = '\0';
    
    return length;
}

int printk(uint8_t color, char *str, ...)
{
    va_list list;
    char buffer[80];

    char opt;
    uint32_t num;
    int is_signed, base;
    int error = 0;

    int read = 0;

    va_start(list, str);

    while (*str) {
        if (*str != '%') {
            putc(color, *str++);
            continue;
        }
        
        // Lettura dei comandi 
        str++;
        opt = *str++;

        if (opt == 's') {
            char *p = va_arg(list, char *);
            print_pm(color, p);
            continue;
        }

        is_signed = 0;
        switch (opt) {
            case 'x':
                base = 16;
                break;
            
            case 'd':
                is_signed = 1;
                __fallthrough;
            case 'u':
                base = 10;
                break;

            case 'b':
                base = 2;
                break;
            
            case 'o':
                base = 8;
                break;
            
            default:
                return read;
        }

        num = va_arg(list, uint32_t);
        error = base_convert(buffer, 80, is_signed ? (uint32_t)ABS((int32_t)num) : num, base) <= 0;
        if (error)
            return read;
        
        if (is_signed && (int32_t)num < 0)
            putc(color, '-');
        print_pm(color, buffer);
        read++;
    }

    va_end(list);
    return read;
}