    .code32
    .text
    
    .global page_directory_load
    .global paging_enable

page_directory_load:
    push %ebp
    mov %esp, %ebp

    mov 8(%esp), %eax
    mov %eax, %cr3

    mov %ebp, %esp
    pop %ebp
    ret

paging_enable:
    push %ebp
    mov %esp, %ebp

    mov %cr0, %eax
    or $0x80000001, %eax
    mov %eax, %cr0

    mov %ebp, %esp
    pop %ebp
    ret
    