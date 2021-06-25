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

#endif