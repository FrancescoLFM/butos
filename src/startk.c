#include <include/asm.h>
#include <libs/print.h>

#include <cpu/idt.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <libs/alloc.h>

void vga_open();
void vga_scroll_down();
void test_ata();

void test_string();
void test_allocator();

void _start()
{
    void *heap_start = (void *) 0x200000;
    size_t heap_size = 10000;
    void *registry_start = heap_start + heap_size;
    size_t registry_capacity = 100 * 8; /* numero massimo di allocazioni */

    isr_install();
    keyboard_start();
    vga_open();
    kalloc_start(
        heap_start,
        heap_size,
        registry_start,
        registry_capacity
    );

    stop();
}
