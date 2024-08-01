#ifndef CPU_IDT_H
#define CPU_IDT_H

#include <include/def.h>

#define ISR_NUM         32
#define IRQ_NUM         16
#define IRQ(num)        ((ISR_NUM) + (num))
#define IDT_ENTRIES     255
#define BUTOS_INT       0x80

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
