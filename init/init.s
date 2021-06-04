    .code16
    .text
    .include    "../macro.s"

    .global init
init:
    xor     %ax, %ax
    mov     %ax, %ds
    mov     %ax, %es
    ljmp    $0, $_start
_start:
    mov     $start_string, %bx
    call    puts
    
    movb    %dl, (drive_number)

    mov     $set_cursor, %ah
    xor     %dl, %dl
    xor     %dh, %dh
    xor     %bx, %bx
    int     $video_int

    mov     $0x0002, %cx
    xor     %dh, %dh
    mov     $0x7e00, %bx
    call    read_sector

    call    clear
    jmp     %bx

end:
    hlt
    jmp     end 

start_string:
    .ascii "Butos is starting..."

    .global drive_number
drive_number:
    .byte   0x00
