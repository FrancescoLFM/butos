#include <include/asm.h>
#include <libs/print.h>

#include <cpu/idt.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>

void vga_open();
void vga_scroll_down();

void _start()
{
    isr_install();
    init_kb();
    vga_open();


    for (int i = 0; i < VGA_ROWS; i++) {
        printk(RED, "%d", i);
        if (i+1 < VGA_ROWS) putc(STD_COLOR, '\n');
    }

    stop();
}
