/* BUTOS SYSCALL COLLECTION */
/* NOT POSIX COMPLIANT (FOR NOW)*/

#include <drivers/vga.h>
#include <libs/print.h>
#include <libs/scan.h>
#include <libs/alloc.h>
#include <cpu/idt.h>
#include <cpu/proc.h>
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
    char *output = (char *) regs->ebx;

    *output = (char) getchar();
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

void exec_handler(struct registers_t *regs)
{
    char *path = (char *) regs->ebx;
    int *exit_code_ptr = (int *) regs->ecx;
    int exit_code = 0;

    /* The caller's image is unmapped while the child runs, write back only after */
    regs->eax = (uint32_t) process_spawn(path, &exit_code);
    if (exit_code_ptr != NULL)
        *exit_code_ptr = exit_code;
}

void puts_color_handler(struct registers_t *regs)
{
    puts_c((uint8_t) regs->ebx, (char *) regs->ecx);
}