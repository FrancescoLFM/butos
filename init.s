     .code16
    .text
    .include    "macro.s"
    
    .global init
init:
    xor     %ax, %ax
    mov     %ax, %ds
    ljmp    $0, $_start
_start:
    call    clear

    mov     $set_cursor, %ah
    xor     %dl, %dl
    xor     %dh, %dh
    xor     %bx, %bx
    int     $video_int

    call    read
end:
    hlt
    jmp     end 

#    .include    "write.s"
#    .include    "read.s"

