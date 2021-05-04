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
    
    push    %ax
    mov     $0x0E, %ah
    int     $0x10
    pop     %ax

    # Delete
    cmp     $0x0E08, %ax
    jne     read_L0
    call    delete

    jmp     read_continue

    # Enter
read_L0:
    cmp     $0x1C0D, %ax
    jne     read_L1
    
    call    newline

read_L1:

read_continue:
    or      %al, %al
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

# Non va un cazzo madonna troia bastarda
# Buggata come la merda
delete:
start_f
    pusha
    # jmp     3f
    call    is_first_column
    test    %dl, %dl
    jz      3f

    mov     $0x02, %ah
    xor     %bh, %bh
    mov     $0x50, %dl
    dec     %dh
    int     $0x10

1:
    mov     $0x08, %ah
    int     $0x10

    cmp     $0x20, %al
    jne     4f
2:    
    call    del_char
    jmp     1b

3:
    call    del_char

4:
    popa
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
