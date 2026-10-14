    .code16
    .include "include/macro.s"

    .text

# cx:  LBA low word (sector number)
# ax:  number of sectors to read
# es:bx: destination buffer
# The read is split in chunks of at most read_chunk sectors and the buffer
# is normalized (offset < 16) so a chunk never crosses a 64K segment
# (the BIOS would wrap the offset and overwrite low memory).
    .equ    read_chunk, 0x40

    .global read_sector
read_sector:
start_f
    pusha
    movw    $0x0010, (dap)
    movw    %cx, (dap + 8)
    mov     %bx, %si
    and     $0x0F, %si
    movw    %si, (dap + 4)
    mov     %bx, %si
    shr     $4, %si
    mov     %es, %di
    add     %di, %si
    movw    %si, (dap + 6)
    mov     %ax, %cx

next_chunk:
    jcxz    end
    mov     %cx, %bx
    cmp     $read_chunk, %bx
    jbe     0f
    mov     $read_chunk, %bx
0:
    movw    %bx, (dap + 2)
    mov     $0x03, %di

1:
    mov     (drive_number), %dl
    movb    $0x42, %ah
    mov     $dap, %si
    int     $disk_int
    jnc     2f

    dec     %di
    jz      4f
    xor     %ah, %ah
    int     $disk_int
    jmp     1b

2:
    sub     %bx, %cx
    addw    %bx, (dap + 8)
    shl     $5, %bx
    addw    %bx, (dap + 6)
    jmp     next_chunk

4:
    movzx   %ah, %dx
    push    %dx
    movw    $panic_msg, %bx
    call    puts
    pop     %dx
    call    printh
3:
    hlt
    jmp     3b

end:
    popa
end_f

    .data
panic_msg:
    .asciz "Disk error code: "
dap:
    .space 16
