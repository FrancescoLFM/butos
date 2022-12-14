#ifndef DEFINE_H
#define DEFINE_H

#include <stdint.h>

#define IN
#define OUT

#define NULL    ((void*)(0))

typedef unsigned long int size_t;

typedef unsigned long int   u32;
typedef unsigned short int  u16;
typedef unsigned char       u8;


// attributes

#define __fallthrough   __attribute__((fallthrough))
#define __packed        __attribute__((packed))
#define force_inline    __attribute__((always_inline)) inline

#define IN_BOUNDS(LEFT, A, RIGHT)   ((LEFT) < (A) && (A) < (RIGHT))
#define IN_BOUNDS_EQ(LEFT, A, RIGHT) ((LEFT) <= (A) && (A) <= (RIGHT))

#define ABS(A)  ((A) >= 0 ? (A) : -(A))
#define SGN(A)  ((A) >= 0 ? 1 : -1)

#define ARR_SIZE(arr)   ((sizeof(arr)) / (sizeof(arr[0])))

#define LOW_OFFSET(off)     ((off) & 0xff)
#define HIGH_OFFSET(off)    (((off) >> 8) & 0xff)
#define low_16(addr)        (uint16_t)(((addr) & 0xFFFF));
#define high_16(addr)       (uint16_t)(((addr) >> 16) & 0xFFFF);

#define BIT(N)              (0x01 << (N)) 


#endif