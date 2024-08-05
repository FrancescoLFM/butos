.code16

.equ    screen_size, 0x1950
.equ    last_column, 0x4f

.equ    set_cursor, 0x02
.equ    teletype, 0x0E

.equ    video_int, 0x10
.equ    disk_int, 0x13
.equ    kb_int, 0x16

.equ    bootloader_start, 0x07E00
.equ    bootloader_size, 0x50

.equ    kernel_start, 0xf000
.equ    kernel_head, 0x01
.equ    kernel_sector, 0x04
.equ    kernel_relocated_mem, 0x100000

.macro start_f
    push    %bp
    mov     %sp, %bp
.endm

# leave is equivalent to:
#    mov     %bp, %sp
#    pop     %bp

.macro end_f
    leave
    ret
.endm

