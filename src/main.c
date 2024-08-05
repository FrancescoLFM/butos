#include <include/asm.h>
#include <libs/print.h>

#include <cpu/idt.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <libs/alloc.h>
#include <cpu/proc.h>

void test_allocator();

void main()
{
    isr_install();
    vga_open();
    vga_clear(BLACK);

    void *heap_start = (void *) 0xC00f0000;
    size_t heap_size = 492032;
    void *registry_start = (unsigned char *)heap_start + heap_size;
    size_t registry_capacity = 100 * 8;

    kalloc_start(
        heap_start,
        heap_size,
        registry_start,
        registry_capacity
    );

    keyboard_start(100);

    test_allocator();

    stop();
}
