#include <include/print.h>
#include <include/asm.h>
#include <stdarg.h>

static struct vga_char *vga_pointer = VGA_TEXT_START;

/*
static void* get_abs_cursor(void* base, uint8_t size, struct cursor crs, uint8_t cols)
{
    return base + (crs.y * cols + crs.x) * size;
}

static void mv_cursor(struct cursor *crs, uint8_t x, uint8_t y)
{
    crs->x = x;
    crs->y = y;
}

static void mv_vga_pointer(struct cursor crs)
{
    if (crs.x < VGA_COLS && crs.y < VGA_ROWS)
        vga_pointer = get_abs_cursor(VGA_TEXT_START, sizeof(struct vga_char), crs, VGA_COLS);
}
*/

static struct vga_char* inc_vga_pointer(uint16_t pos)
{
    vga_pointer = &vga_pointer[pos];
    return vga_pointer;
}

static struct vga_char* vga_newline() { return inc_vga_pointer(VGA_COLS); }

static int vga_allign_left()
{
    uint32_t temp;
    uint8_t mod;
    
    temp = ((uint32_t)(vga_pointer) - 0xb8000);
    mod = temp % (VGA_COLS * sizeof(struct vga_char));
    temp -= mod;

    vga_pointer = (struct vga_char*)((temp) + 0xb8000);
    
    return mod >> 1;
}

static int vga_tab()
{
    uint32_t temp;
    uint8_t mod;
    size_t tab_size = ((VGA_COLS >> 3) * sizeof(struct vga_char));
    
    temp = ((uint32_t)(vga_pointer) - 0xb8000);
    mod = temp % tab_size;
    temp += tab_size - mod;

    vga_pointer = (struct vga_char*)((temp) + 0xb8000);
    
    return (tab_size - mod) >> 1;
}

static struct vga_char* dec_vga_pointer(uint16_t pos)
{
    vga_pointer -= pos;
    return vga_pointer;
}

static int __putc(uint8_t color, char c) {
    switch (c) {
    case '\n':
        vga_newline();
        return VGA_COLS;
    
    case '\b':
        dec_vga_pointer(1);
        vga_pointer->ascii = ' ';
        vga_pointer->color = color;
        return -1;
    
    case '\r':
        return -vga_allign_left();
    
    case '\t':
        return vga_tab();
    
    case 0:
        return 0;
    
    default:
        vga_pointer->ascii = c;
        vga_pointer->color = color;
        inc_vga_pointer(1);
    }

    return 1;
}

int putc(uint8_t color, char c)
{
    if (!c)
        return 0;
    
    int relpos = __putc(color, c);

    inc_cursor(relpos);
    return relpos;
}

int print_pm(uint8_t color, char* string)
{
    int count = 0;
    int relpos = 0;

    while (*string) {
        relpos += __putc(color, *string++);
        count++;
    }

    inc_cursor(relpos);

    return count;
}

int puts(char* str)
{
    return print_pm(STD_COLOR, str);
}

char* htos(char* buffer, uint32_t hex, uint8_t size)
{
    char* digits = "0123456789abcdef";
    uint8_t nibbles = size << 1;

    for (int i = 0; i < nibbles; i++) {
        buffer[nibbles - (i + 1)] = digits[hex & 0x0F];
        hex >>= 4;
    }

    buffer[nibbles] = 0;

    return buffer;
}

int printk(uint8_t color, char *str, ...)
{
    char buffer[9];
    uint32_t tp;

    va_list list;

    va_start(list, str);

    while (*str) {
        if (*str != '%') {
            putc(color, *str++);
        } else {
            str++;
            switch (*str++) {
            case 'x':
                tp = va_arg(list, uint32_t);
                htos(buffer, tp, sizeof(uint32_t));
                print_pm(color, buffer);
                break;
            case 0:
                return -1;
            }
        }
    }

    va_end(list);
    return 0;
}

void disable_cursor()
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    // select register 0x0A (cursor start register)
    outb(0x3D4, 0x0A);

    // write Cursor Scan Line Start
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

    outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

static void __update_cursor(uint16_t pos)
{
    outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void update_cursor(const struct cursor crs)
{
	uint16_t pos = crs.y * VGA_WIDTH + crs.x;
	__update_cursor(pos);
}

static uint16_t __get_cursor_position()
{
    uint16_t pos = 0;

    outb(0x3D4, 0x0F);
    pos |= inb(0x3D5);
    outb(0x3D4, 0x0E);
    pos |= ((uint16_t)inb(0x3D5)) << 8;

    return pos;
}

struct cursor* get_cursor_position(struct cursor* crs)
{
    uint16_t pos = __get_cursor_position();

    crs->x = pos % VGA_WIDTH;
    crs->y = (pos - crs->x) / VGA_WIDTH;

    return crs;
}

void inc_cursor(uint8_t pos)
{
    __update_cursor(__get_cursor_position() + pos);
}

void dec_cursor(uint8_t pos)
{
    __update_cursor(__get_cursor_position() - pos);
}