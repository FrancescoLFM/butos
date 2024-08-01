    .code32
    .text

    .global isrs
    .global irqs
    .global butos_int

isr_common:
    pusha
    mov     %ds, %eax
    push    %eax

    push    %esp
    call    isr_handler
    
    add     $0x08, %esp
    popa
    add     $0x08, %esp
    
stop:
    hlt
    jmp     stop

isr0:
    push    $0x00
    push    $0x00
    jmp     isr_common
isr1:
    push    $0x00
    push    $0x01
    jmp     isr_common
isr2:
    push    $0x00
    push    $0x02
    jmp     isr_common
isr3:
    push    $0x00
    push    $0x03
    jmp     isr_common
isr4: 
    push    $0x00
    push    $0x04
    jmp     isr_common
isr5:
    push    $0x00
    push    $0x05
    jmp     isr_common
isr6:
    push    $0x00
    push    $0x06
    jmp     isr_common
isr7:
    push    $0x00
    push    $0x07
    jmp     isr_common
isr8:
    push    $0x08
    jmp     isr_common
isr9:
    push    $0x00
    push    $0x09
    jmp     isr_common
isr10:
    push    $0x0a
    jmp     isr_common
isr11:
    push    $0x0b
    jmp     isr_common
isr12:
    push    $0x0c
    jmp     isr_common
isr13:
    push    $0x0d
    jmp     isr_common
isr14:
    push    $0x0e
    jmp     isr_common
isr15:
    push    $0x00
    push    $0x0f
    jmp     isr_common
isr16:
    push    $0x00
    push    $0x10
    jmp     isr_common
isr17:
    push    $0x00
    push    $0x11
    jmp     isr_common
isr18:
    push    $0x00
    push    $0x12
    jmp     isr_common
isr19:
    push    $0x00
    push    $0x13
    jmp     isr_common
isr20:
    push    $0x00
    push    $0x14
    jmp     isr_common
isr21:
    push    $0x00
    push    $0x15
    jmp     isr_common
isr22:
    push    $0x00
    push    $0x16
    jmp     isr_common
isr23:
    push    $0x00
    push    $0x17
    jmp     isr_common
isr24:
    push    $0x00
    push    $0x18
    jmp     isr_common
isr25:
    push    $0x00
    push    $0x19
    jmp     isr_common
isr26:
    push    $0x00
    push    $0x1a
    jmp     isr_common
isr27:
    push    $0x00
    push    $0x1b
    jmp     isr_common
isr28:
    push    $0x00
    push    $0x1c
    jmp     isr_common
isr29:
    push    $0x00
    push    $0x1d
    jmp     isr_common
isr30:
    push    $0x00
    push    $0x1e
    jmp     isr_common
isr31:
    push    $0x00
    push    $0x1f
    jmp     isr_common

irq_common:
    pusha
    mov     %ds, %eax
    push    %eax

    push    %esp
    call    irq_handler
    add     $0x08, %esp
    popa
    add     $0x08, %esp
    iret
    
irq0:
    push    $0x0
    push    $0x20
    jmp     irq_common
irq1:
    push    $0x1
    push    $0x21
    jmp     irq_common
irq2:
    push    $0x2
    push    $0x22
    jmp     irq_common
irq3:
    push    $0x3
    push    $0x23
    jmp     irq_common
irq4:
    push    $0x4
    push    $0x24
    jmp     irq_common
irq5:
    push    $0x5
    push    $0x25
    jmp     irq_common
irq6:
    push    $0x6
    push    $0x26
    jmp     irq_common
irq7:
    push    $0x7
    push    $0x27
    jmp     irq_common
irq8:
    push    $0x8
    push    $0x28
    jmp     irq_common
irq9:
    push    $0x9
    push    $0x29
    jmp     irq_common
irq10:
    push    $0xa
    push    $0x2a
    jmp     irq_common
irq11:
    push    $0xb
    push    $0x2b
    jmp     irq_common
irq12:
    push    $0xc
    push    $0x2c
    jmp     irq_common
irq13:
    push    $0xd
    push    $0x2d
    jmp     irq_common
irq14:
    push    $0xe
    push    $0x2e
    jmp     irq_common
irq15:
    push    $0xf
    push    $0x2f
    jmp     irq_common

syscall_common:
    pusha
    mov     %ds, %eax
    push    %eax

    push    %esp
    sti
    call    syscall_handler
    add     $0x08, %esp
    popa
    add     $0x08, %esp
    iret

software_int:
    push    $0x00
    push    $0x80
    jmp     syscall_common
    .data
isrs:
    .long isr0
    .long isr1
    .long isr2
    .long isr3
    .long isr4
    .long isr5
    .long isr6
    .long isr7
    .long isr8
    .long isr9
    .long isr10
    .long isr11
    .long isr12
    .long isr13
    .long isr14
    .long isr15
    .long isr16
    .long isr17
    .long isr18
    .long isr19
    .long isr20
    .long isr21
    .long isr22
    .long isr23
    .long isr24
    .long isr25
    .long isr26
    .long isr27
    .long isr28
    .long isr29
    .long isr30
    .long isr31

irqs:
    .long irq0
    .long irq1
    .long irq2
    .long irq3
    .long irq4
    .long irq5
    .long irq6
    .long irq7
    .long irq8
    .long irq9
    .long irq10
    .long irq11
    .long irq12
    .long irq13
    .long irq14
    .long irq15

butos_int:
    .long software_int