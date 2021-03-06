#ifndef ASM_H
#define ASM_H

#include <stdint.h>
#include <include/def.h>

#define TEST(value, mask)   ((value) & (mask))

static force_inline uint8_t inb(uint16_t port)
{
    uint8_t ret;

    asm volatile ("in %%dx, %%al"
                 : "=a" (ret)
                 : "d"  (port)
                 );
    
    return ret;
}

static force_inline uint16_t inw(uint16_t port)
{
    uint16_t ret;

    asm volatile ("in %%dx, %%ax"
                 : "=a" (ret)
                 : "d"  (port)
                 );
    
    return ret;
}

static force_inline void outb(uint16_t port, uint8_t value)
{
    asm volatile ("out %%al, %%dx"
                 :
                 : "a" (value), "d" (port)
                 );
}

static force_inline void outw(uint16_t port, uint16_t value)
{
    asm volatile ("out %%ax, %%dx"
                 :
                 : "a" (value), "d" (port)
                 );
}

static force_inline void hlt() { asm volatile ("hlt"); }

static force_inline void stop() { for (;;) asm volatile ("hlt"); }

static force_inline void io_delay(void)
{
    const uint16_t DELAY_PORT = 0x80;
    asm volatile("outb %%al,%0" : : "dN" (DELAY_PORT));
}

static force_inline uint32_t get_rip()
{
    uint32_t rip;

    asm volatile ("call 1f;"
                  "1: pop %0"
                 : "=r" (rip)
                 );
    
    return rip;
}

static force_inline void set_interrupts() { asm volatile ("sti"); }

static force_inline void clear_interrupts() { asm volatile ("cli"); }

#endif