#ifndef ASM_H
#define ASM_H

#include <stdint.h>

static __attribute__((always_inline)) inline uint8_t inb(uint16_t port)
{
    uint8_t ret;

    asm volatile ("in %%dx, %%al"
                 : "=a" (ret)
                 : "d"  (port)
                 );
    
    return ret;
}

static __attribute__((always_inline)) inline uint16_t inw(uint16_t port)
{
    uint16_t ret;

    asm volatile ("in %%dx, %%ax"
                 : "=a" (ret)
                 : "d"  (port)
                 );
    
    return ret;
}

static __attribute__((always_inline)) inline void outb(uint16_t port, uint8_t value)
{
    asm volatile ("out %%al, %%dx"
                 :
                 : "a" (value), "d" (port)
                 );
}

static __attribute__((always_inline)) inline void outw(uint16_t port, uint16_t value)
{
    asm volatile ("out %%ax, %%dx"
                 :
                 : "a" (value), "d" (port)
                 );
}

static __attribute__((always_inline)) inline void hlt() { asm volatile ("hlt"); }

static __attribute__((always_inline))  inline void stop() { for (;;) asm volatile ("hlt"); }

#endif