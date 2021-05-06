    .code16
    .text
    .include    "macro.s"
    
    .global init
init:
    xor     %ax, %ax
    mov     %ax, %ds
    ljmp    $0, $_start
_start:
# Clear the screen changing the video mode
    call    clear
# Stampa il prompt butos a schermo
# Buggato, al momento della cancellazione muore tutto e viene cancellata la prima lettera
    # print_prompt
# Legge comandi
    call    read
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

