#include "vga.h"

static struct vga_char *vga_pointer = VGA_TEXT_START;

void* get_abs_cursor(void* base, uint8_t size, struct cursor crs, uint8_t cols)
{
    return base + (crs.y * cols + crs.x) * size;
}

void mv_cursor(struct cursor *crs, uint8_t x, uint8_t y)
{
    crs->x = x;
    crs->y = y;
}

void mv_vga_pointer(struct cursor crs)
{
    if (crs.x < VGA_COLS && crs.y < VGA_ROWS)
        vga_pointer = get_abs_cursor(VGA_TEXT_START, sizeof(struct vga_char), crs, VGA_COLS);
}

struct vga_char* get_vga_pointer() { return vga_pointer; }

struct vga_char* inc_vga_pointer(uint16_t pos)
{
    vga_pointer = &vga_pointer[pos];
    return vga_pointer;
}

struct vga_char* vga_newline() { return inc_vga_pointer(VGA_COLS); }

struct vga_char* vga_allign_left()
{
    uint32_t temp;
    
    temp = ((uint32_t)(vga_pointer) - 0xb8000);
    temp -= temp % (VGA_COLS * sizeof(struct vga_char));
    vga_pointer = (struct vga_char*)((temp) + 0xb8000 - 2);
    
    return vga_pointer;
}

struct vga_char* dec_vga_pointer(uint16_t pos)
{
    vga_pointer -= sizeof(struct vga_char) * pos;
    return vga_pointer;
}

void print_pm(int color, char* string)
{
    do {
        switch (*string) {
        case '\n':
            vga_newline();
            break;
        
        case '\r':
            vga_allign_left();
            break;
        
        default:
            vga_pointer->ascii = *string;
            vga_pointer->color = color;
            inc_vga_pointer(1);
        }
    } while (*string++);
}

#include "asm.h"

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

void __update_cursor(uint16_t pos)
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

uint16_t __get_cursor_position()
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