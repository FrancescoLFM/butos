#include <cpu/paging.h>
#include <include/asm.h>

#define STACK_BOTTOM 0xC00f0000

force_inline void relocate_stack(uint32_t addr) {
    __asm__ __volatile__ (
        "mov %0, %%esp \n"
        "mov %0, %%ebp"
        :
        : "r"(addr)
    );
}

/* Second stage bootloader */
void _start()
{
    paging_init();

    // Relocating stack to 0xC000f000
    relocate_stack(STACK_BOTTOM);
    jmp(KERNEL_VIRTUAL_START);
}
