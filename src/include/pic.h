#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC1            0x20
#define PIC2            0xA0
#define COMMAND         0x00
#define DATA            0x01

// Queste altre sono prese da OSDEV
#define ICW1_ICW4       0x01
#define ICW1_SINGLE     0x02
#define ICW1_INTERVAL4  0x04
#define ICW1_LEVEL      0x08
#define ICW1_INIT       0x10
 
#define ICW4_8086       0x01
#define ICW4_AUTO       0x02
#define ICW4_BUF_SLAVE  0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM       0x10

#define EOI             0x20

#define LOW_OFFSET(off)     ((off) & 0xff)
#define HIGH_OFFSET(off)    (((off) >> 8) & 0xff)

void pic_send_command(uint8_t pic, uint8_t command);

void restart_all_pics();

void pic_send_data(uint8_t pic, uint8_t data);

void remap_pic(uint8_t offset1, uint8_t offset2, uint16_t mask);

void end_of_interrupt(uint8_t pic);

#endif