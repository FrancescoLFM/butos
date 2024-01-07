#include <include/asm.h>
#include <libs/print.h>

#include <cpu/idt.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <libs/alloc.h>

void test_ata();
void test_string();
void test_allocator();
void test_scan();

void _start()
{
    void *heap_start = (void *) 0x200000;
    size_t heap_size = 100000;
    void *registry_start = (unsigned char *)heap_start + heap_size;
    size_t registry_capacity = 100 * 8;

    kalloc_start(
        heap_start,
        heap_size,
        registry_start,
        registry_capacity
    );

    isr_install();
    keyboard_start(100);
    vga_open();

    vga_clear(BLACK);

    test_scan();

    stop();
}
