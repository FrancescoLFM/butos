#include <stdint.h>
#include <include/asm.h>
#include <cpu/idt.h>
#include <cpu/pic.h>
#include <libs/print.h>

extern const uint32_t volatile isrs[ISR_NUM];
extern const uint32_t volatile irqs[IRQ_NUM];

struct idt_entry {
    uint16_t offset_1;
    uint16_t selector; // should be set to 0x08 (see GDT)
    uint8_t zero;      // unused
    uint8_t type_attr;
    uint16_t offset_2;
} __attribute__((packed));

struct idt_entry volatile idt[UINT8_MAX];

void add_idt_gate(uint8_t n, uint32_t handler)
{
    idt[n].offset_1     = low_16(handler);
    idt[n].offset_2     = high_16(handler);
    idt[n].selector     = 0x08;                 // 0b10001110 = 0x8E
    idt[n].type_attr    = 0x8E;
    idt[n].zero         = 0x00;
}

void isr_install() 
{
    for (int i = 0; i < ISR_NUM; i++)
        add_idt_gate(i, isrs[i]);

    remap_pic(ISR_NUM, ISR_NUM + 8, 0x0000);

    for (int i = 0; i < IRQ_NUM; i++)
        add_idt_gate(i + ISR_NUM, irqs[i]);

    load_idt();

    set_interrupts();
}

char *isr_msg[] = {
    "Can't divide By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Unexpected page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check"
};

void isr_handler(struct registers_t *regs)
{
    puts("CPU exception: ");

    if(regs->isr_no < ARR_SIZE(isr_msg))
        puts(isr_msg[regs->isr_no]);
    else
        puts("Reserved");
}

struct idt_descriptor{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void load_idt()
{
    struct idt_descriptor volatile idt_desc;
    idt_desc.base = (uint32_t) idt;
    idt_desc.limit = IDT_ENTRIES * (sizeof(struct idt_entry)) - 1;
    __asm__ __volatile__ (
        "lidt (%0)" 
        :
        : "r" (&idt_desc)
    );
}

isr_t int_handler[UINT8_MAX] = {0};

void irq_handler(struct registers_t *regs)
{
    isr_t handler;

    if (int_handler[regs->isr_no]) {
        handler = int_handler[regs->isr_no];
        handler(regs);
    }
    
    end_of_interrupt(PIC1);
    if (regs->isr_no < 40)
        end_of_interrupt(PIC2);
}

void set_int_handler(uint8_t n, isr_t handler) {
    int_handler[n] = handler;
}
