.code16
.include "../macro.s"

.global clear
.global get_cursor_position

get_cursor_position:
    mov     $0x03, %ah
    xor     %bh, %bh
    int     $video_int
    ret

clear:
start_f
    pusha

    mov     $0x0600, %ax
    mov     $0x0F, %bh
    mov     $screen_size, %dx
    xor     %cx, %cx
    int     $video_int
    
    mov     $0x02, %ah
    xor     %dx, %dx
    xor     %bh, %bh
    int     $video_int

    popa
end_f
