    .code16
    .text
    .include    "macro.s"
    
    .global init
init:
    movb    $0x00, (drive_number)
    xor     %ax, %ax
    mov     %ax, %ds
    mov     %ax, %es
    ljmp    $0, $_start
_start:
    call    clear

    mov     $set_cursor, %ah
    xor     %dl, %dl
    xor     %dh, %dh
    xor     %bx, %bx
    int     $video_int

    mov     $0x0002, %cx
    xor     %dh, %dh
    mov     $0x7e00, %bx
    call    read_sector

    call    puts

end:
    hlt
    jmp     end 

    .global drive_number
drive_number:
    .byte   0x00
