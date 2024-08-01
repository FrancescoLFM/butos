#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <cpu/idt.h>
#include <libs/print.h>

#define SYSCALLS_NUM 7

void putc_handler(struct registers_t *regs);
void puts_handler(struct registers_t *regs);
void printf_handler(struct registers_t *regs);
void clear_handler(struct registers_t *regs);
void getchar_handler(struct registers_t *regs);
void kalloc_handler(struct registers_t *regs);
void kfree_handler(struct registers_t *regs);

static void __attribute__((unused)) (*syscall_handlers[SYSCALLS_NUM])(struct registers_t *) = {putc_handler, puts_handler, printf_handler, 
                                                                                               clear_handler, getchar_handler, kalloc_handler, 
                                                                                               kfree_handler};

/* 
    ? Butos syscalls collection
    Francesco Pallara 2024 - <francescopallara.pa@gmail.com>
    You can include this header from here in every butos program. 
*/
#include <stdarg.h>
#include <include/def.h>

/* PRINT SYSCALLS */

///@brief Display a single character
///@param EAX: Putc syscall num (0)
///@param EBX: Character to display
static force_inline void putc_syscall(char c) 
{
    __asm__ __volatile__ (
        "mov $0, %%eax \n"
        "mov %0, %%ebx \n"
        "int $0x80"
        :
        : "r"((uintptr_t) c) 
        : "eax", "ebx"
    );
}

///@brief Display a C string
///@param EAX: Puts syscall num (1)
///@param EBX: String to display
static force_inline void puts_syscall(char *s) 
{
    __asm__ __volatile__ (
        "mov $1, %%eax \n"
        "mov %0, %%ebx \n"
        "int $0x80"
        :
        : "r"((uintptr_t) s) 
        : "eax", "ebx"
    );
}

///@brief Display a formatted C string
///@param EAX: Puts syscall num (2)
///@param EBX: Pointer to variadic list
static __attribute__((unused)) void printf_syscall(char *s, ...) 
{
    va_list list;
    va_start(list, s);

    __asm__ __volatile__ (
        "mov $2, %%eax \n"
        "mov %0, %%ebx \n"
        "mov %1, %%ecx \n"
        "int $0x80"
        :
        : "r"((uintptr_t) s), "r"((uintptr_t) &list)
        : "eax", "ebx", "ecx"
    );

    va_end(list);
}

///@brief Clears the screen and change color
///@param EAX: Clear syscall num (3)
///@param EBX: Screen color
static force_inline void clear_syscall(uint8_t color) 
{
    __asm__ __volatile__ (
        "mov $3, %%eax \n"
        "mov %0, %%ebx \n"
        "int $0x80"
        :
        : "r"((uintptr_t) color) 
        : "eax", "ebx"
    );
}

/* INPUT SYSCALLS */

///@brief Get character from keyboard
///@param EAX: Getchar syscall num (4)
///@param EBX: Pointer to return character
static force_inline void getchar_syscall(char *c) 
{
    __asm__ __volatile__ (
        "mov $4, %%eax \n"
        "mov %0, %%ebx \n"
        "int $0x80 \n"
        :  
        : "r"((uintptr_t)c)
        : "eax", "ebx"
    );
}

/* HEAP MEMORY SYSCALLS */

///@brief Alloc heap kernel memory
///@param EAX: Kalloc syscall num (5)
///@param EBX: Pointer to buffer
///@param ECX: Buffer size
static force_inline void kalloc_syscall(void **buff, size_t size) 
{
    __asm__ __volatile__ (
        "mov $5, %%eax \n"
        "mov %0, %%ebx \n"
        "mov %1, %%ecx \n"
        "int $0x80 \n"
        :  
        : "r"((uintptr_t)buff), "r"(size)
        : "eax", "ebx"
    );
}

///@brief Free heap kernel memory
///@param EAX: Kfree syscall num (6)
///@param EBX: Pointer to buffer
static force_inline void kfree_syscall(void *buff) 
{
    __asm__ __volatile__ (
        "mov $6, %%eax \n"
        "mov %0, %%ebx \n"
        "int $0x80 \n"
        :  
        : "r"((uintptr_t)buff)
        : "eax", "ebx"
    );
}

#endif