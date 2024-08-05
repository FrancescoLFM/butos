    .code16
    .text
    .include    "include/macro.s"

    .global init
init:
    xor     %ax, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %ss
    mov     %ax, %gs
    mov     %ax, %fs

    mov     $0x7000, %bp
    mov     %bp, %sp

    ljmp    $0, $_start
_start:
    # Reading the kernel bootloader from the disk
    movb    %dl, (drive_number)
    mov     $0x0003, %cx
    mov     $bootloader_size, %al
    xor     %dh, %dh
    mov     $bootloader_start, %bx
    call    read_sector
    # Reading the kernel from the disk
    movb    %dl, (drive_number)
    mov     $kernel_sector, %cx
    mov     $kernel_head, %dh
    mov     (num_sectors), %al
    mov     $kernel_start, %bx
    call    read_sector

    cli
    lgdt    gdt_descriptor

    # Let's go in 32 bits protected mode
    mov     %cr0, %eax
    or      $0x01, %al
    mov     %eax, %cr0

    ljmp    $0x08, $init_protected
    
    .code32
init_protected:
    movl    $(0x0100 * SEC), %ecx
    movl    $kernel_start, %esi
    movl    $kernel_relocated_mem, %edi
    rep movsl
    # Initializing register for the protected mode
    mov     $0x10, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs
    mov     %ax, %ss

    xor     %eax, %eax
    mov     %eax, %ebx
    mov     %eax, %ecx
    mov     %eax, %edx
    jmp     bootloader_start
    .data

    .global drive_number
drive_number:
    .byte   0x00

num_sectors:
    .byte   SEC
