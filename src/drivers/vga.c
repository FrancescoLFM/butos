#include <include/def.h>
#include <drivers/vga.h>

struct vga_char {
    char    ascii;
    uint8_t color;
} __packed;

static struct vga_char *vga_pointer = VGA_TEXT_START;

static const struct vga_char *MIN_VID = VGA_TEXT_START;
static const struct vga_char *MAX_VID = VGA_TEXT_START + (VGA_ROWS * VGA_COLS);

int vga_inc_pointer(int pos)
{

    if (!IN_BOUNDS(MIN_VID, vga_pointer + pos, MAX_VID))
        pos = 0;
    
    vga_pointer += pos;

    return pos;
}

void vga_write(uint8_t color, char c)
{
    vga_pointer->color = color;
    vga_pointer->ascii = c;
}

int vga_newline()
{
    return vga_inc_pointer(VGA_COLS);
}

int vga_allign_left()
{
    uint32_t temp;
    uint8_t mod;
    
    temp = (uint32_t)(vga_pointer - VGA_TEXT_START);
    mod = temp % VGA_COLS;
    temp -= mod;

    vga_pointer = VGA_TEXT_START + temp;
    
    return mod;
}

int vga_tab()
{
    uint8_t mod;
    size_t tab_size;
    
    /* Divido lo spazio in otto colonne */
    tab_size = VGA_COLS / 8;
    mod = (vga_pointer - VGA_TEXT_START) % tab_size;
    
    return vga_inc_pointer(tab_size - mod);
}