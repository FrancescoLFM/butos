# Code by Giovanni Zaccaria
# Copyright 2021
# This product is under MIT License

    .code16
    .include "../macro.s"

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
    movzx   %ah, %dx
    call    printh
3:
    hlt
    jmp     3b

end:
    popa
end_f
