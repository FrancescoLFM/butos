#ifndef IDT_H
#define IDT_H

#define low_16(addr)    (uint16_t)(((addr) & 0xFFFF));
#define high_16(addr)   (uint16_t)(((addr) >> 16) & 0xFFFF);

#define ISR_NUM         32
#define IRQ_NUM         16
#define IRQ(num)        ((ISR_NUM) + (num))
#define IDT_ENTRIES     ((ISR_NUM) + (IRQ_NUM))

void isr_install();

void load_idt();

struct registers_t {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t isr_no, error_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

// pointer to void function with registers_t* argument
typedef void (*isr_t)(struct registers_t *);

void set_int_handler(uint8_t n, isr_t handler);

#endif
