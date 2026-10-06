#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <cpu/idt.h>
#include <libs/print.h>

#define SYSCALLS_NUM 16

void putc_handler(struct registers_t *regs);
void puts_handler(struct registers_t *regs);
void printf_handler(struct registers_t *regs);
void clear_handler(struct registers_t *regs);
void getchar_handler(struct registers_t *regs);
void kalloc_handler(struct registers_t *regs);
void kfree_handler(struct registers_t *regs);
void exec_handler(struct registers_t *regs);
void puts_color_handler(struct registers_t *regs);
void open_handler(struct registers_t *regs);
void close_handler(struct registers_t *regs);
void read_handler(struct registers_t *regs);
void write_handler(struct registers_t *regs);
void seek_handler(struct registers_t *regs);
void create_handler(struct registers_t *regs);
void remove_handler(struct registers_t *regs);

static void __attribute__((unused)) (*syscall_handlers[SYSCALLS_NUM])(struct registers_t *) = {putc_handler, puts_handler, printf_handler, 
                                                                                               clear_handler, getchar_handler, kalloc_handler, 
                                                                                               kfree_handler, exec_handler, puts_color_handler,
                                                                                               open_handler, close_handler, read_handler,
                                                                                               write_handler, seek_handler, create_handler,
                                                                                               remove_handler};

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
        : "eax", "ebx", "memory"
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
        : "eax", "ebx", "ecx", "memory"
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

/* PROCESS SYSCALLS */

/* exec_syscall return values */
#define EXEC_OK             0
#define EXEC_NOT_FOUND      1
#define EXEC_INVALID        2
#define EXEC_FAILED         3

///@brief Run the program at path and wait for it to exit
///@param EAX: Exec syscall num (7), on return the exec status
///@param EBX: Program path
///@param ECX: Pointer to the exit code (nullable)
static force_inline int exec_syscall(char *path, int *exit_code) 
{
    int status;

    __asm__ __volatile__ (
        "mov $7, %%eax \n"
        "mov %1, %%ebx \n"
        "mov %2, %%ecx \n"
        "int $0x80 \n"
        : "=&a"(status)
        : "r"((uintptr_t)path), "r"((uintptr_t)exit_code)
        : "ebx", "ecx", "memory"
    );

    return status;
}

/* PRINT SYSCALLS (continued) */

///@brief Display a C string with a color
///@param EAX: Puts color syscall num (8)
///@param EBX: Color
///@param ECX: String to display
static force_inline void puts_color_syscall(uint8_t color, char *s) 
{
    __asm__ __volatile__ (
        "mov $8, %%eax \n"
        "mov %0, %%ebx \n"
        "mov %1, %%ecx \n"
        "int $0x80"
        :
        : "r"((uintptr_t) color), "r"((uintptr_t) s) 
        : "eax", "ebx", "ecx"
    );
}

/* FILE SYSCALLS */

/* Error codes returned (negative) by the file syscalls */
#define FILE_OK             0
#define FILE_NOT_FOUND      -1
#define FILE_INVALID        -2
#define FILE_EXISTS         -3
#define FILE_TOO_MANY       -4
#define FILE_BUSY           -5
#define FILE_IO_ERROR       -6

/* seek_syscall whence */
#define SEEK_SET            0
#define SEEK_CUR            1
#define SEEK_END            2

///@brief Open an existing file
///@param EAX: Open syscall num (9), on return the file descriptor or an error
///@param EBX: Absolute file path
static force_inline int open_syscall(char *path) 
{
    int ret;

    __asm__ __volatile__ (
        "mov $9, %%eax \n"
        "mov %1, %%ebx \n"
        "int $0x80 \n"
        : "=&a"(ret)
        : "r"((uintptr_t)path)
        : "ebx", "memory"
    );

    return ret;
}

///@brief Close a file descriptor, writing the changes to the disk
///@param EAX: Close syscall num (10), on return the status
///@param EBX: File descriptor
static force_inline int close_syscall(int fd) 
{
    int ret;

    __asm__ __volatile__ (
        "mov $10, %%eax \n"
        "mov %1, %%ebx \n"
        "int $0x80 \n"
        : "=&a"(ret)
        : "r"(fd)
        : "ebx", "memory"
    );

    return ret;
}

///@brief Read from the current position of a file
///@param EAX: Read syscall num (11), on return the bytes read (0 at end of file) or an error
///@param EBX: File descriptor
///@param ECX: Buffer
///@param EDX: Bytes to read
static force_inline int read_syscall(int fd, void *buff, size_t size) 
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(11), "b"(fd), "c"((uintptr_t)buff), "d"(size)
        : "memory"
    );

    return ret;
}

///@brief Write at the current position of a file, growing it if needed
///@param EAX: Write syscall num (12), on return the bytes written or an error
///@param EBX: File descriptor
///@param ECX: Buffer
///@param EDX: Bytes to write
static force_inline int write_syscall(int fd, void *buff, size_t size) 
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(12), "b"(fd), "c"((uintptr_t)buff), "d"(size)
        : "memory"
    );

    return ret;
}

///@brief Move the position of a file, at most to its end
///@param EAX: Seek syscall num (13), on return the new position or an error
///@param EBX: File descriptor
///@param ECX: Offset
///@param EDX: Whence (SEEK_SET, SEEK_CUR, SEEK_END)
static force_inline int seek_syscall(int fd, int offset, int whence) 
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(13), "b"(fd), "c"(offset), "d"(whence)
        : "memory"
    );

    return ret;
}

///@brief Create an empty file, the name must be 8.3
///@param EAX: Create syscall num (14), on return the status
///@param EBX: Absolute file path
static force_inline int create_syscall(char *path) 
{
    int ret;

    __asm__ __volatile__ (
        "mov $14, %%eax \n"
        "mov %1, %%ebx \n"
        "int $0x80 \n"
        : "=&a"(ret)
        : "r"((uintptr_t)path)
        : "ebx", "memory"
    );

    return ret;
}

///@brief Delete a file that is not open
///@param EAX: Remove syscall num (15), on return the status
///@param EBX: Absolute file path
static force_inline int remove_syscall(char *path) 
{
    int ret;

    __asm__ __volatile__ (
        "mov $15, %%eax \n"
        "mov %1, %%ebx \n"
        "int $0x80 \n"
        : "=&a"(ret)
        : "r"((uintptr_t)path)
        : "ebx", "memory"
    );

    return ret;
}

#endif
