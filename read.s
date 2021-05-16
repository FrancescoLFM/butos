    .code16
    .include "macro.s"

    .equ    screen_size, 0x1950
    .equ    last_column, 0x4f

    .text

    .global read
read:
start_f
    pusha
read_loop:
    xor     %ah, %ah
    int     $kb_int

    cmp     $0x07, %al  # Ascii control
    jle     delete

    push    %ax
    mov     $teletype, %ah
    int     $video_int
    pop     %ax

delete:
    cmp     $0x0E08, %ax
    jne     read_L0
    
    call    del_char
    dec     %bx
    jmp     1f

    # Enter
read_L0:
    cmp     $0x1C0D, %ax
    jne     read_L1
    
    call    newline
    jmp     read_exit

read_L1:
    cmp     $0x4B00, %ax
    jne     read_L2

    call    move_cursor_left
    dec     %bx
    jmp     1f

read_L2:
    cmp     $0x4D00, %ax
    jne     read_continue

    call    move_cursor_right
    jmp     read_continue

read_continue:
    inc     %bx
1:
    mov     %al, (%bx)
    jmp     read_loop

read_exit:
    inc     %bx
    movb     $0x00, (%bx)

    popa
end_f

# return dl = 1 (yes), 0(no)
# return dh = row
is_first_column:
start_f
    push    %ax
    push    %bx
    push    %cx

    xor     %al, %al

    call    get_cursor_position

    test    %dh, %dh
    jz      1f

    test    %dl, %dl
    jnz     1f

    mov     $0x01, %al
1:
    mov     %al, %dl

    test    %dl, %dl
    
    pop     %cx
    pop     %bx
    pop     %ax
end_f

# Se siamo all'ultima riga e all'ultima colonna segnala che si deve fermare
# [z] non lo siamo
# [nz] lo siamo
# [ff in dl] se siamo alla fine del mondo
is_last_column:
start_f
    push    %ax
    push    %bx
    push    %cx

    call    get_cursor_position

    xor     %al, %al

    cmp     $last_column, %dl
    jne     1f

    # Check if last column of last row
    cmp     $0x19, %dh
    jl      0f      

    mov     $0xff, %dl
    jmp     1f

0:
    mov     $0x01, %al

1:
    test    %al, %al    # set zero flag
    jmp     2f
2:
    pop    %ax
    pop    %bx
    pop    %cx  
    
end_f

# Probabile broken muovendo cx
del_char:
start_f
    push    %cx
    mov     $0x0A, %ah
    mov     $0x20, %al
    mov     $1, %cx
    int     $video_int
    pop     %cx
end_f

    .global clear
clear:
start_f
    pusha

    mov     $0x07, %ah
    mov     $0x0F, %bh
    mov     $screen_size, %dx
    xor     %cx, %cx
    int     $video_int
    
    mov     $0x02, %ah
    xor     %dx, %dx
    xor     %bh, %bh
    int     $video_int

    popa
end_f

newline:
start_f
    pusha

    call    get_cursor_position

    # Set new cursor position
    mov     $0x02, %ah
    inc     %dh
    xor     %dl, %dl
    int     $video_int

    popa
end_f

# Il programma non arriva qui
move_cursor_left:
start_f
    pusha

    xor     %bh, %bh
    call    is_first_column
    jnz     1f

    mov     $teletype, %ah
    mov     $0x08, %al

    jmp     2f
1:
    # first column
    mov     $0x02, %ah
    dec     %dh
    mov     $last_column, %dl

2:
    int     $video_int
    popa
end_f

get_cursor_position:
    mov     $0x03, %ah
    xor     %bh, %bh
    int     $video_int
    ret

move_cursor_right:
start_f
    pusha

    call    is_last_column
    jz      1f

    xor     %dl, %dl
    inc     %dh
    
    jmp     2f

    # Set current curson position
    # Qui è da sistemare usando due registri (%dl %dh)
1:
    inc     %dl
    
2:
    mov     $0x02, %ah
    int     $video_int
    popa
end_f
