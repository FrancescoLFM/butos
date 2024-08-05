/* BUTOS SYSCALL COLLECTION */
/* NOT POSIX COMPLIANT (FOR NOW)*/

#include <drivers/vga.h>
#include <libs/print.h>
#include <libs/scan.h>
#include <libs/alloc.h>
#include <cpu/idt.h>
#include <include/asm.h>
#include <stdarg.h>

void putc_handler(struct registers_t *regs)
{
    char c = (char) regs->ebx;
    putc(c);
}

void puts_handler(struct registers_t *regs)
{
    char *c = (char *) regs->ebx;
    puts(c);
}

void printf_handler(struct registers_t *regs)
{
    char *str = (char *) regs->ebx;
    va_list *list_ptr = (va_list *) regs->ecx;
    
    vprintk_c(STD_COLOR, str, *list_ptr);
}

void clear_handler(struct registers_t *regs)
{
    vga_clear((uint8_t) regs->ebx);
}

void getchar_handler(struct registers_t *regs)
{
    char c = getchar();
    uint32_t *output = (uint32_t *) regs->ebx;

    *output = (uint32_t) c;
}

void kalloc_handler(struct registers_t *regs)
{
    void **buff_ptr = (void **) regs->ebx;
    size_t buff_size = regs->ecx;
    
    *buff_ptr = kalloc(buff_size);
}

void kfree_handler(struct registers_t *regs)
{
    kfree((void *) regs->ebx);
}