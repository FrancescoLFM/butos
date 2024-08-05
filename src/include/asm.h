#ifndef ASM_H
#define ASM_H

#include <stdint.h>
#include <include/def.h>

#define TEST(value, mask)   ((value) & (mask))

static force_inline uint8_t inb(uint16_t port)
{
    uint8_t ret;

    __asm__ __volatile__ (
        "in %%dx, %%al"
        : "=a" (ret)
        : "d"  (port)
    );
    
    return ret;
}

static force_inline uint16_t inw(uint16_t port)
{
    uint16_t ret;

    __asm__ __volatile__ (
        "inw %%dx, %%ax"
        : "=a" (ret)
        : "d"  (port)
    );
    
    return ret;
}

static force_inline uint32_t inl(uint16_t port)
{
    uint32_t ret;

    __asm__ __volatile__ (
        "in %%dx, %%eax"
        : "=a" (ret)
        : "d"  (port)
    );
    
    return ret;
}

static force_inline void outb(uint16_t port, uint8_t value)
{
    __asm__ __volatile__ (
        "out %%al, %%dx"
        :
        : "a" (value), "d" (port)
    );
}

static force_inline void outw(uint16_t port, uint16_t value)
{
    __asm__ __volatile__ (
        "out %%ax, %%dx"
        :
        : "a" (value), "d" (port)
    );
}

static force_inline void outl(uint16_t port, uint32_t value)
{
    __asm__ __volatile__ (
        "out %%eax, %%dx"
        :
        : "a" (value), "d" (port)
    );
}

static force_inline void hlt() { __asm__ __volatile__ ("hlt"); }

static force_inline void stop() { for (;;) __asm__ __volatile__ ("hlt"); }

static force_inline void io_delay(void)
{
    const uint16_t DELAY_PORT = 0x80;
    __asm__ __volatile__("outb %%al,%0" : : "dN" (DELAY_PORT));
}

static force_inline uint32_t get_rip()
{
    uint32_t rip;

    __asm__ __volatile__ (
        "call 1f;"
        "1: pop %0"
        : "=r" (rip)
    );
    
    return rip;
}

static force_inline void jmp(uintptr_t absolute_addr) { 
    __asm__ __volatile__ (
        "jmp *%0"
        :
        : "r"(absolute_addr)
    ); 
}

static force_inline void set_interrupts() { __asm__ __volatile__ ("sti"); }

static force_inline void clear_interrupts() { __asm__ __volatile__ ("cli"); }

static force_inline void iret() { __asm__ __volatile__ ("iret"); }

#endif
