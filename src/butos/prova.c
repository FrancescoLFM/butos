#include <include/asm.h>
#include <libs/print.h>

#include <cpu/idt.h>
#include <drivers/keyboard.h>

void _start()
{
    isr_install();
    init_kb();

    printk(STD_COLOR, "prova\n");
    printk(STD_COLOR, "%d\n", 18);
    stop();
}
