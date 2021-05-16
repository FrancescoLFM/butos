    .code16
    .include "macro.s"

    .text

    .global write
# bx = char* string
# cx = uint16_t size
write:
start_f
    pusha
    mov     $0x0E, %ah
1:
    mov     (%bx), %al
    int    $0x10
    inc     %bx
    loop    1b
    popa
end_f

    .global puts
# bx = char* string
puts:
start_f
    pusha
    
    mov     %bx, %si
    mov     $0x0E, %ah
puts_loop:
    lodsb
    test    %al, %al
    jz      puts_exit
    int     $0x10
    jmp     puts_loop

puts_exit:
    popa 
end_f

    .global htos
# dx = Number to convert
htos:
start_f
    pusha
    mov     $0x04, %cx
    mov     $hex_string, %di
1:
    mov     %dl, %al
    and     $0x0f, %al
    mov     %al, %ah
    
    add     $0x30, %al
    cmp     $0x0a, %ah
    jl      2f
    add     $0x07, %al
2:
    mov     %cx, %bx
    mov     %al, 1(%bx, %di) # tipo bx + cx + 0x01
    shr     $0x04, %dx
    loop    1b
    popa
end_f

    .global printh
# dx = Number to convert
printh:
start_f
    push    %bx

    call    htos

    mov     $hex_string, %bx
    call    puts

    pop     %bx
end_f

hex_string:
    .asciz  "0x0000" #7c7d
