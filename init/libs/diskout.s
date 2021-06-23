# Code by Giovanni Zaccaria
# Copyright 2021
# This product is under MIT License

    .code16
    .include "include/macro.s"

    .text

# (es) bx: pointer to buffer
# ch: cylinder (0)
# dh: head     (0)
# cl: sector   (1)
# al: number of sectors to read
    .global read_sector
read_sector:
start_f
    pusha
    push    %ax
    mov     (drive_number), %dl
    mov     $0x03, %si

1:
    mov     $0x02, %ah
    int     $disk_int
    jnc     end

    dec     %si
    jz      2f
    xor     %ah, %ah
    int     $disk_int
    jmp     1b

2:
    mov     $panic_msg, %bx
    call    puts
    movzx   %ah, %dx
    call    printh
3:
    hlt
    jmp     3b

end:
    pop     %dx
    cmp     %dl, %al
    jne     2b
    popa
end_f

panic_msg:
    .asciz "Disk error, system halted. Error code: "
