#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <include/colors.h>

/* IOAS */

#define IOAS_BIT            0x01
#define SELECT_MODE(IOAS)   ((IOAS) ? 0x0D0 : 0x0B0)

/* GLOBAL REGISTERS */

#define VGA_MISCELLANEOUS_OUTPUT_REGISTER_READ      0x3CC
#define VGA_MISCELLANEOUS_OUTPUT_REGISTER_WRITE     0x3C2
#define VGA_FEATURE_CONTROL_REGISTER_READ           0x3CA
#define VGA_FEATURE_CONTROL_REGISTER_WRITE(IOAS)    (0x30A | SELECT_MODE(IOAS))
#define VGA_INPUT_STATUS_0_REGISTER                 0x3C2
#define VGA_INPUT_STATUS_1_REGISTER(IOAS)           (0x30A | SELECT_MODE(IOAS))

#define VGA_CRTC_ADDRESS(IOAS)    (0x304 | SELECT_MODE(IOAS))
#define VGA_CRTC_DATA(IOAS)       (0x305 | SELECT_MODE(IOAS))

/* INTERNAL REGISTERS */

/* documentation: http://www.osdever.net/FreeVGA/vga/crtcreg.htm#00 */

#define VGA_HORIZONTAL_TOTAL_REGISTER               0x00
#define VGA_END_HORIZONTAL_DISPLAY_REGISTER         0x01
#define VGA_START_HORIZONTAL_BLANKING_REGISTER      0x02
#define VGA_END_HORIZONTAL_BLANKING_REGISTER        0x03
#   define EVRA                                     BIT(7)
#   define DISPLAY_ENABLE_SKEW                      (BIT(6) | BIT(5))
#   define END_HORIZONTAL_BLANKING                  0b11111
#define VGA_START_HORIZONTAL_RETRACE_REGISTER       0x04
#define VGA_END_HORIZONTAL_RETRACE_REGISTER         0x05
#   define  EBH5                                    BIT(7)
#   define  HORIZONTAL_RETRANCE_SKEW                (BIT(6) | BIT(5))
#   define  END_HORIZONTAL_RETRACE                  0b11111
#define VGA_VERTICAL_TOTAL_REGISTER                 0x06
#define VGA_OVERFLOW_REGISTER                       0x07
#   define  VRS9                                    BIT(7)
#   define  VDE9                                    BIT(6)
#   define  VT9                                     BIT(5)
#   define  LC8                                     BIT(4)
#   define  SVB8                                    BIT(3)
#   define  VRS8                                    BIT(2)
#   define  VDE8                                    BIT(1)
#   define  VT8                                     BIT(0)
#define VGA_PRESET_ROW_SCAN_REGISTER                0x08
#   define  BYTE_PANNING                            (BIT(6) | BIT(5))
#   define  PRESET_ROW_SCAN                         0b11111
#define VGA_MAXIMUM_SCAN_LINE_REGISTER              0x09
#   define  SD                                      BIT(7)
#   define  LC9                                     BIT(6)
#   define  SVB9                                    BIT(5)
#   define  MAXIMUM_SCAN_LINE                       0b11111
#define VGA_CURSOR_START_REGISTER                   0x0A
#   define  CD                                      BIT(5)
#   define  CURSOR_SCAN_LINE_START                  0b11111
#define VGA_CURSOR_END_REGISTER                     0x0B
#   define  CURSOR_SKEW                             (BIT(6) | BIT(5))
#   define  CURSOR_SCAN_LINE_END                    0b111111    /* 0b011111 + il bit 5 dello skew!*/
#define VGA_START_ADDRESS_HIGH_REGISTER             0x0C
#define VGA_START_ADDRESS_LOW_REGISTER              0x0D
#define VGA_CURSOR_LOCATION_HIGH_REGISTER           0x0E
#define VGA_CURSOR_LOCATION_LOW_REGISTER            0x0F
#define VGA_VERTICAL_RETRACE_START_REGISTER         0x10
#define VGA_VERTICAL_RETRACE_END_REGISTER           0x11
#   define  PROTECT                                 BIT(7)
#   define  BANDWIDTH                               BIT(6)
#   define  VERTICAL_RETRACE_END                    0b1111
#define VGA_VERTICAL_DISPLAY_END_REGISTER           0x12
#define VGA_OFFSET_REGISTER                         0x13
#define VGA_UNDERLINE_LOCATION_REGISTER             0x14
#   define  DW                                      BIT(6)
#   define  DIV4                                    BIT(5)
#   define  UNDERLINE_LOCATION                      0b11111
#define VGA_START_VERTICAL_BLANKING_REGISTER        0x15
#define VGA_END_VERTICAL_BLANKING                   0x16
#define VGA_CRTC_MODE_CONTROL_REGISTER              0x17
#   define  SE                                      BIT(7)
#   define  WORD_BYTE                               BIT(6)
#   define  AW                                      BIT(5)
#   define  DIV2                                    BIT(3)
#   define  SLDIV                                   BIT(2)
#   define  MAP14                                   BIT(1)
#   define  MAP13                                   BIT(0)
#define VGA_LINE_COMPARE_REGISTER                   0x18

/* VIDEO MEMORY */

#define VGA_TEXT_START  ((struct vga_char *)(0xb8000))
#define VGA_TEXT_END    ((struct vga_char *)(0xb9f40))
#define VGA_COLS        80
#define VGA_WIDTH       VGA_COLS
#define VGA_ROWS        25

struct cursor {
    uint8_t x;
    uint8_t y;
};

void vga_open();
int vga_pointer_inc(int pos);
void vga_write(uint8_t color, char c);
int vga_newline();
int vga_allign_left();
int vga_tab();
void vga_scroll_down();

/* REGISTERS */

void vga_register_select(uint8_t reg);
uint8_t vga_register_read();
void vga_register_write(uint8_t data);

/* CURSOR */

void vga_cursor_disable();
void vga_cursor_enable(uint8_t cursor_start, uint8_t cursor_end);
uint16_t vga_cursor_get_position_();
void vga_cursor_update_(uint16_t pos);
void vga_cursor_update(const struct cursor crs);
struct cursor* vga_cursor_get_position(struct cursor* crs);
void vga_cursor_inc(int pos);

#endif