    # Origin = 0x0600
    .code16
    .text

self_relocation:
    cli
    xor     %ax, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %ss
    mov     %ax, %gs
    mov     %ax, %fs
copy_loop:
    movw    $0x0100, %cx
    movw    $0x7c00, %si
    movw    $0x0600, %di
    rep movsw
    
    ljmp $0, $read_PT1

read_PT1:
    sti
    mov     $2, %ah
    mov     $1, %al
    mov     $2, %cx
    xor     %dh, %dh
    mov     $0x7c00, %bx
    int     $0x13
    ljmp    $0, $0x7c00

    .data
UID:
    .long 0
RESERVED:
    .word 0
PT1:
    .byte 0x80 # Active partition
    .word 0    # Cylinder, head 0
    .byte 2    # Second sector
    .byte 1    # Random partition type
    .word 1    # Cylinder 0, head 1
    .byte 247  # 100kb
    .long 1
    .long 202

PT2:
    .byte 0x80 # Active partition
    .word 0
    .byte 0
    .byte 0xC  # FAT32 with lba
    .word 0
    .byte 0
    .long 203
    .long 65312

