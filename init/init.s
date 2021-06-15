    .code16
    .text
    .include    "../macro.s"

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
    movb    %dl, (drive_number)

    mov     $0x0002, %cx
    mov     (num_sectors), %al
    xor     %dh, %dh
    mov     $0xf000, %bx
    call    read_sector

    call    clear

# Enabling A20 Line
    #mov     $0x2401, %ax
    #int     $0x15

    #call    enable_a20

    inb	    $0x92
	andb	$(~0x03), %al
	orb	    $0x02, %al
	outb	$0x92

    cli
    lgdt    gdt_descriptor

    # Let's go in 32 bits protected mode
    mov     %cr0, %eax
    or      $0x01, %al
    mov     %eax, %cr0

    ljmp    $0x08, $init_protected
    
    .code32
init_protected:
    mov     $0x10, %ax
    mov     %ax, %ds
    
    jmp     0xf000

    .data

    .global drive_number
drive_number:
    .byte   0x00

num_sectors:
    .byte   0x07
