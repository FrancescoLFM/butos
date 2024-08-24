#include <include/asm.h>
#include <libs/print.h>

#include <cpu/idt.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <libs/alloc.h>
#include <cpu/proc.h>
#include <cpu/paging.h>

void test_allocator();
void elf_test();

void main()
{
    page_directory_adjust();
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

    elf_test();

    stop();
}
