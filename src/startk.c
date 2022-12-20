#include <include/asm.h>
#include <libs/print.h>

#include <cpu/idt.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>

void vga_open();
void vga_scroll_down();
void test_ata();

void _start()
{
    isr_install();
    init_kb();
    vga_open();

    test_ata();

    stop();
}
