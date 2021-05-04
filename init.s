    .code16
    .text
    .include    "macro.s"
    
    .global init
init:
    xor     %ax, %ax
    mov     %ax, %ds
    ljmp    $0, $_start
_start:
    call clear
    call read
end:
    nop
    nop
    nop
    nop
    nop
    nop
    hlt
    jmp     end 

    .include    "write.s"
    .include    "read.s"

string:
    .ascii  "Lanfredi gay"
    .byte   0x0a, 0x0d, 0x00
len:
    .byte   (.-string)
