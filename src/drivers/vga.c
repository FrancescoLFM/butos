#include <include/def.h>
#include <include/asm.h>
#include <libs/string.h>
#include <drivers/vga.h>

struct vga_char {
    char    ascii;
    uint8_t color;
} __packed;

static uint8_t vga_ioas = 0;

static struct vga_char *vga_pointer = VGA_TEXT_START;

static const struct vga_char *MIN_VID = VGA_TEXT_START;
static const struct vga_char *MAX_VID = VGA_TEXT_START + (VGA_ROWS * VGA_COLS);

void vga_open()
{
    vga_ioas = inb(VGA_MISCELLANEOUS_OUTPUT_REGISTER_READ) & IOAS_BIT;
    vga_cursor_enable(13, 14);
}

void vga_pointer_set(struct vga_char *address)
{
    if (!IN_BOUNDS(MIN_VID-1, address, MAX_VID))
        return;
    
    vga_pointer = address;
    vga_cursor_update_(address - VGA_TEXT_START);
}

int vga_pointer_inc(int pos)
{
    if (!IN_BOUNDS(MIN_VID-1, vga_pointer + pos, MAX_VID))
        pos = 0;
    
    vga_pointer += pos;
    vga_cursor_inc(pos);

    return pos;
}

void vga_write(uint8_t color, char c)
{
    vga_pointer->color = color;
    vga_pointer->ascii = c;
}

int vga_newline()
{
    int pos;
    
    pos = vga_pointer_inc(VGA_COLS);
    if (pos == 0)
        vga_scroll_down();
    
    return pos;
}

int vga_allign_left()
{
    uint32_t temp;
    uint8_t mod;
    
    temp = (uint32_t)(vga_pointer - VGA_TEXT_START);
    mod = temp % VGA_COLS;

    vga_pointer_inc(-mod);
    
    return -mod;
}

int vga_tab()
{
    uint8_t mod;
    size_t tab_size;
    
    /* Divido lo spazio in otto colonne */
    tab_size = VGA_COLS / 8;
    mod = (vga_pointer - VGA_TEXT_START) % tab_size;
    
    return vga_pointer_inc(tab_size - mod);
}

void vga_scroll_down()
{
    struct vga_char *lineptr;

    for (lineptr = VGA_TEXT_START; lineptr <= VGA_TEXT_END - 80; lineptr += VGA_COLS)
        memcpy(lineptr, lineptr + VGA_COLS, sizeof(*lineptr) * 80);

}

/* REGISTERS */

void vga_register_select(uint8_t reg)
{
    outb(VGA_CRTC_ADDRESS(vga_ioas), reg);
}

uint8_t vga_register_read()
{
    return inb(VGA_CRTC_DATA(vga_ioas));
}

void vga_register_write(uint8_t data)
{
    outb(VGA_CRTC_DATA(vga_ioas), data);
}

/* CURSOR */

void vga_cursor_disable()
{
    vga_register_select(VGA_CURSOR_START_REGISTER);
    vga_register_write(CD);
}

void vga_cursor_enable(uint8_t cursor_start, uint8_t cursor_end)
{
    uint8_t old;

    vga_register_select(VGA_CURSOR_START_REGISTER);
    
    old = vga_register_read();
    vga_register_write((old & ~(CURSOR_SCAN_LINE_START)) | cursor_start);

    vga_register_select(VGA_CURSOR_START_REGISTER);
    old = vga_register_read();
    vga_register_write((old & ~(CURSOR_SCAN_LINE_END)) | cursor_end);
}

void vga_cursor_update_(uint16_t pos)
{
    vga_register_select(VGA_CURSOR_LOCATION_LOW_REGISTER);
    vga_register_write((uint8_t)(LOW_OFFSET(pos)));

	vga_register_select(VGA_CURSOR_LOCATION_HIGH_REGISTER);
	vga_register_write((uint8_t)HIGH_OFFSET(pos));
}

void vga_cursor_update(const struct cursor crs)
{
	uint16_t pos = crs.y * VGA_WIDTH + crs.x;
	vga_cursor_update_(pos);
}

uint16_t vga_cursor_get_position_()
{
    uint16_t pos = 0;

    vga_register_select(VGA_CURSOR_LOCATION_LOW_REGISTER);
    pos |= vga_register_read();
    vga_register_select(VGA_CURSOR_LOCATION_HIGH_REGISTER);
    pos |= (uint16_t)vga_register_read() << 8;

    return pos;
}

struct cursor* vga_cursor_get_position(struct cursor* crs)
{
    uint16_t pos = vga_cursor_get_position_();

    crs->x = pos % VGA_WIDTH;
    crs->y = (pos - crs->x) / VGA_WIDTH;

    return crs;
}

void vga_cursor_inc(int pos)
{
    vga_cursor_update_(vga_cursor_get_position_() + pos);
}