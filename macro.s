.equ    set_cursor, 0x02
.equ    teletype, 0x0E

.equ    video_int, 0x10
.equ    kb_int, 0x16

.macro start_f
    push    %bp
    mov     %sp, %bp
.endm

.macro end_f
    mov     %bp, %sp
    pop     %bp
    ret
.endm

#! da sistemare il cursore
.macro print_prompt
    push    %bx
    mov     $string, %bx
    call    puts
    pop     %bx
.endm
