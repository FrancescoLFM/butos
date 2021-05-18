    .code16
    .include "macro.s"

    .text

# (es) bx: pointer to buffer
# ch: cylinder (0)
# dh: head     (0)
# cl: sector   (1)
    .global read_sector
read_sector:
start_f
    pusha
    mov     $0x0201, %ax
    mov     (drive_number), %dl
    int     $disk_int
    jnc     end

    movzx   %ah, %dx
    call    printh
end:
    popa
end_f