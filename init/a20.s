    .code16
    .include "../macro.s"

    .text

.equ    status_port, 0x64
.equ    port_a, 0x60
.equ    buf_full, 0x02

.macro  io_delay
    jmp     .+2
.endm

    .global enable_a20
enable_a20:
start_f
    mov     $empty_8042_failed, %bx
    cmpb    $0x01, (%bx)
    jz      1f

    call    empty_8042
    jnz     1f

    mov     $0xd1, %al
    outb    %al, $status_port
    call    empty_8042
    jnz     1f

    mov     $0xdf, %al
    outb    %al, $port_a
    call    empty_8042

    mov     $0xff, %al
    outb    %al, $status_port
    call    empty_8042

1:
end_f

empty_8042:
start_f
    pusha
    xor     %cx, %cx
1:
    inb     $status_port, %al
    io_delay
    io_delay
    io_delay
    io_delay
    and     $buf_full, %al
    loopnz  1b

    cmp     $0x00, %cx
    jnz     2f

    mov     $empty_8042_failed, %bx
    mov     $0x01, (%bx)
2:
    and     $buf_full, %al
    popa
end_f

    .data
empty_8042_failed:
    .byte   0x00
