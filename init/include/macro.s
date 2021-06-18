.code16

.equ    screen_size, 0x1950
.equ    last_column, 0x4f

.equ    set_cursor, 0x02
.equ    teletype, 0x0E

.equ    video_int, 0x10
.equ    disk_int, 0x13
.equ    kb_int, 0x16

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

