    .code16

    .equ    screen_size, 0x1950

    .text

    .global read

read:
start_f
read_loop:
    pusha
    xor     %ah, %ah
    int     $0x16

    cmp     $0x07, %al
    jle     delete

    push    %ax
    mov     $0x0E, %ah
    int     $0x10
    pop     %ax

delete:
    cmp     $0x0E08, %ax
    jne     read_L0
    call    del_char

    jmp     read_continue

    # Enter
read_L0:
    cmp     $0x1C0D, %ax
    jne     read_L1
    
    call    newline

read_L1:
    cmp     $0x4B00, %ax
    jne     read_L2

    call    move_cursor_left

read_L2:
    cmp     $0x4D00, %ax
    jne     read_continue

    call    move_cursor_right

read_continue:
    or      %ax, %ax
    jnz     read_loop

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

    mov     $0x03, %ah
    xor     %bh, %bh
    int     $0x10

    test    %dh, %dh
    jz      1f

    test    %dl, %dl
    jnz     1f

    mov     $0x01, %al
1:
    mov     %al, %dl
    
    pop     %cx
    pop     %bx
    pop     %ax
end_f

del_char:
start_f
    mov     $0x0A, %ah
    mov     $0x20, %al
    mov     $1, %cx
    int     $0x10
end_f


clear:
start_f
    pusha

    mov     $0x07, %ah
    mov     $0x0F, %bh
    mov     $screen_size, %dx
    xor     %cx, %cx
    int     $0x10
    
    mov     $0x02, %ah
    xor     %dx, %dx
    xor     %bh, %bh
    int     $0x10

    popa
end_f

newline:
start_f
    pusha

    # Get cursor position
    mov     $0x03, %ah
    xor     %bh, %bh
    int     $0x10

    # Set new cursor position
    mov     $0x02, %ah
    inc     %dh
    xor     %dl, %dl
    int     $0x10

    popa
end_f

# Il programma non arriva qui
move_cursor_left:
start_f
push    %ax

    mov     $0x0E, %ah
    mov     $0x08, %al
    int     $0x10

pop     %ax
end_f

move_cursor_right:
start_f
pusha

    # Get cursor position
    mov     $0x03, %ah
    xor     %bh, %bh
    int     $0x10

    # Set current curson position
    inc     %dl
    mov     $0x02, %ah
    int     $0x10
    
popa
end_f
