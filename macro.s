.macro start_f
    push    %bp
    mov     %sp, %bp
.endm

.macro end_f
    mov     %bp, %sp
    pop     %bp
    ret
.endm
