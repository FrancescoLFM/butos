#include <libs/print.h>
#include <drivers/vga.h>
#include <include/asm.h>
#include <stdarg.h>

#define MIN_BASE    2
#define MAX_BASE    16

static char* digits = "0123456789abcdef";

int putc_c(uint8_t color, int c)
{
    switch (c) {
    case '\n':
        vga_newline();
        __fallthrough;
    
    case '\r':
        vga_allign_left();
        break;

    case '\b':
        if (!vga_pointer_inc(-1))
            break;
        vga_write(color, ' ');
        break;
    
    case '\t':
        vga_tab();
        break;

    case 0:
        break;
    
    default:
        vga_write(color, (unsigned char) c);
        vga_pointer_inc(1);
        break;
    }

    return (unsigned char) c;
}

int puts_c(uint8_t color, const char* string)
{
    int count = 0;

    while (*string) {
        putc_c(color, *string++);
        count++;
    }

    return count;
}

int putc(int c)
{
    return (unsigned char) putc_c(STD_COLOR, c);
}

int puts(const char* str)
{
    return puts_c(STD_COLOR, str);
}

/**
 * Convert number @num to base @base into a
 * string which lives in @buffer at most
 * large @size bytes
 *
 * stop when digits are exhausted or if recursion
 * depth exceeds the buffer size
 *
 * each recursion handles the digit depth
 */
static int base_convert_r(char *buffer, size_t size, uint32_t num, uint32_t base)
{
    int depth;

    if (num == 0)
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

    /* base range check */
    if (!IN_BOUNDS_EQ(MIN_BASE, base, MAX_BASE))
        return -1;
    
    /* minimum buffer size check */
    if (size < 2)
        return -1;
    
    /* handling zero */
    if (num == 0) {
        buffer[0] = digits[0];
        buffer[1] = '\0';
        return 1;
    }

    /* recursive conversion */
    length = base_convert_r(buffer, size-1, num, base);
    if (length < 0)
        return -1;
    
    buffer[length] = '\0';
    
    return length;
}

int vprintk_c(uint8_t color, char *str, va_list list)
{
    char buffer[80];

    char opt;
    uint32_t num;
    int is_signed, base;
    int error = 0;

    int read = 0;

    while (*str) {
        if (*str != '%') {
            putc_c(color, *str++);
            continue;
        }
        
        /* lettura dei comandi */
        str++;
        opt = *str++;

        if (opt == 's') {
            char *p = va_arg(list, char *);
            puts_c(color, p);
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

        /* esecuzione dei comandi */
        num = va_arg(list, uint32_t);
        error = base_convert(buffer, 80, is_signed ? (uint32_t)ABS((int32_t)num) : num, base) <= 0;
        if (error)
            return read;
        
        if (is_signed && (int32_t)num < 0)
            putc_c(color, '-');
        puts_c(color, buffer);
        
        read++;
    }

    return read;
}

int printk_c(uint8_t color, char *str, ...)
{
    va_list list;
    int ret;

    va_start(list, str);
    ret = vprintk_c(color, str, list);
    va_end(list);

    return ret;
}

int printk(char *str, ...)
{
    va_list list;
    int ret;

    va_start(list, str);
    ret = vprintk_c(STD_COLOR, str, list);
    va_end(list);

    return ret;
}
