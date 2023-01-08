    .code16
    .include "include/macro.s"

    .text

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
.LC0:
        .string "0123456789abcdef"
htos:
        mov     %sp, %si
        movw    2(%si), %dx
        mov     $3, %ax
.L2:
        # body del for loop
        # buffer[2+i] <- num & 0xf
        # i va da 3 a 0
        mov     %dx, %si
        shr     $4, %dx
        and     $15, %si
        mov     .LC0(%si), %cx
        movb    %cl, hex_string+2(%eax)

        sub     $1, %ax
        jnb     .L2
        mov     $hex_string, %ax
        ret

    .global printh
# dx = Number to convert
printh:
start_f
    push    %bx

	push	%dx
    call    htos
    pop     %dx

    mov     $hex_string, %bx
    call    puts

    pop     %bx
end_f

    .data
hex_string:
    .asciz  "0x0000"
