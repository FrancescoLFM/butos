#include "asm.h"

uint8_t inb(uint16_t port)
{
    uint8_t ret;

    asm volatile ("in %%dx, %%al"
                 : "=a" (ret)
                 : "d"  (port)
                 );
    
    return ret;
}

uint16_t inw(uint16_t port)
{
    uint16_t ret;

    asm volatile ("in %%dx, %%ax"
                 : "=a" (ret)
                 : "d"  (port)
                 );
    
    return ret;
}

void outb(uint16_t port, uint8_t value)
{
    asm volatile ("out %%al, %%dx"
                 :
                 : "a" (value), "d" (port)
                 );
}

void outw(uint16_t port, uint16_t value)
{
    asm volatile ("out %%ax, %%dx"
                 :
                 : "a" (value), "d" (port)
                 );
}

void hlt() { asm volatile ("hlt"); }

void stop() { for (;;) asm volatile ("hlt"); }