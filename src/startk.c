#include <include/asm.h>
#include <libs/print.h>

#include <cpu/idt.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>

void vga_open();
void vga_scroll_down();
void test_ata();

void test_string();

void _start()
{
    isr_install();
    keyboard_start();
    vga_open();

    test_string();

    stop();
}
