#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <cpu/idt.h>
#include <libs/print.h>

#define SYSCALLS_NUM 26

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
void readdir_handler(struct registers_t *regs);
void krealloc_handler(struct registers_t *regs);
void chdir_handler(struct registers_t *regs);
void getcwd_handler(struct registers_t *regs);
void mkdir_handler(struct registers_t *regs);
void rmdir_handler(struct registers_t *regs);
void uname_handler(struct registers_t *regs);
void blkinfo_handler(struct registers_t *regs);
void pipe_handler(struct registers_t *regs);
void keymap_handler(struct registers_t *regs);

static void __attribute__((unused)) (*syscall_handlers[SYSCALLS_NUM])(struct registers_t *) = {putc_handler, puts_handler, printf_handler,
                                                                                               clear_handler, getchar_handler, kalloc_handler,
                                                                                               kfree_handler, exec_handler, puts_color_handler,
                                                                                               open_handler, close_handler, read_handler,
                                                                                               write_handler, seek_handler, create_handler,
                                                                                               remove_handler, readdir_handler, krealloc_handler,
                                                                                               chdir_handler, getcwd_handler, mkdir_handler,
                                                                                               rmdir_handler, uname_handler, blkinfo_handler,
                                                                                               pipe_handler, keymap_handler};

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

///@brief Resize heap kernel memory (NULL ptr = alloc, size 0 = free)
///@param EAX: Krealloc syscall num (17), on return the new pointer (NULL on failure, old block kept)
///@param EBX: Pointer to buffer
///@param ECX: New size
static force_inline void *krealloc_syscall(void *buff, size_t size)
{
    void *ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(17), "b"((uintptr_t)buff), "c"(size)
        : "memory"
    );

    return ret;
}

/* PROCESS SYSCALLS */

/* exec_syscall return values */
#define EXEC_OK             0
#define EXEC_NOT_FOUND      1
#define EXEC_INVALID        2
#define EXEC_FAILED         3
#define EXEC_INVALID_ARGS   4

///@brief Run the program at path as main(argc, argv) and wait for it to exit
///@param EAX: Exec syscall num (7), on return the exec status
///@param EBX: Program path, absolute or relative
///@param ECX: Pointer to the exit code (nullable)
///@param EDX: NULL terminated arguments, program name first (NULL means just path)
///@param ESI: Descriptors the program gets as stdin, stdout, stderr (NULL inherits the caller's ones),
///            EXEC_INVALID_ARGS if one isn't open
static force_inline int exec_syscall(char *path, char **argv, int *stdio, int *exit_code) 
{
    int status;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(status)
        : "a"(7), "b"((uintptr_t)path), "c"((uintptr_t)exit_code), "d"((uintptr_t)argv),
          "S"((uintptr_t)stdio)
        : "memory"
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
#define FILE_NOT_EMPTY      -7

/* Standard streams, always open on the console (keyboard and screen, stderr in red).
   Reading stdin returns a whole line, echoed while typed. They can't be closed. */
#define STDIN_FILENO        0
#define STDOUT_FILENO       1
#define STDERR_FILENO       2

/* seek_syscall whence */
#define SEEK_SET            0
#define SEEK_CUR            1
#define SEEK_END            2

///@brief Open an existing file
///@param EAX: Open syscall num (9), on return the file descriptor or an error
///@param EBX: File path, absolute or relative to the working directory
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
///@param EBX: File path, absolute or relative to the working directory
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
///@param EBX: File path, absolute or relative to the working directory
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

struct dirent {
    char     name[13];
    int      is_dir;
    unsigned int size;
};

///@brief Read directory entry at index (0-based, skips . and ..)
///@param EAX: Readdir syscall num (16), on return 1=found, 0=end, negative=error
///@param EBX: Directory path, absolute or relative
///@param ECX: Entry index
///@param EDX: Pointer to struct dirent to fill
static force_inline int readdir_syscall(char *path, int index, struct dirent *entry)
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(16), "b"((uintptr_t)path), "c"(index), "d"((uintptr_t)entry)
        : "memory"
    );

    return ret;
}

///@brief Change the working directory, relative paths of every syscall start from it
///@param EAX: Chdir syscall num (18), on return the status (FILE_NOT_FOUND, FILE_INVALID if not a directory)
///@param EBX: Directory path, absolute or relative
static force_inline int chdir_syscall(char *path)
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(18), "b"((uintptr_t)path)
        : "memory"
    );

    return ret;
}

///@brief Copy the absolute path of the working directory
///@param EAX: Getcwd syscall num (19), on return its length or FILE_INVALID if it doesn't fit
///@param EBX: Buffer
///@param ECX: Buffer size, terminator included
static force_inline int getcwd_syscall(char *buff, size_t size)
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(19), "b"((uintptr_t)buff), "c"(size)
        : "memory"
    );

    return ret;
}

///@brief Create an empty directory, the name must be 8.3
///@param EAX: Mkdir syscall num (20), on return the status (FILE_EXISTS, FILE_NOT_FOUND if the parent is missing)
///@param EBX: Directory path, absolute or relative
static force_inline int mkdir_syscall(char *path)
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(20), "b"((uintptr_t)path)
        : "memory"
    );

    return ret;
}

///@brief Remove an empty directory
///@param EAX: Rmdir syscall num (21), on return the status (FILE_NOT_EMPTY, FILE_INVALID for files and /,
///            FILE_BUSY if it's the working directory of a running process)
///@param EBX: Directory path, absolute or relative
static force_inline int rmdir_syscall(char *path)
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(21), "b"((uintptr_t)path)
        : "memory"
    );

    return ret;
}

/* SYSTEM INFO SYSCALLS */

#define UTS_LEN             32

struct utsname {
    char sysname[UTS_LEN];
    char nodename[UTS_LEN];
    char release[UTS_LEN];
    char version[UTS_LEN];
    char machine[UTS_LEN];
};

///@brief Get the kernel name, release, build and machine
///@param EAX: Uname syscall num (22), on return the status
///@param EBX: Pointer to struct utsname to fill
static force_inline int uname_syscall(struct utsname *name)
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(22), "b"((uintptr_t)name)
        : "memory"
    );

    return ret;
}

struct blkinfo {
    char         name[8];       /* "hda", "hda1"... */
    char         model[40];     /* Empty for partitions */
    char         mount[16];     /* Empty if not mounted */
    unsigned int start;         /* First LBA */
    unsigned int sectors;       /* 512 bytes sectors */
    unsigned int part_type;     /* MBR partition type, 0 for the whole disk */
    unsigned int bootable;
};

///@brief Read block device at index (0 is the boot disk, then its partitions)
///@param EAX: Blkinfo syscall num (23), on return 1=found, 0=end, negative=error
///@param EBX: Device index
///@param ECX: Pointer to struct blkinfo to fill
static force_inline int blkinfo_syscall(int index, struct blkinfo *info)
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(23), "b"(index), "c"((uintptr_t)info)
        : "memory"
    );

    return ret;
}

/* PIPE SYSCALLS */

///@brief Open an empty pipe: written at its end (up to 64 KiB), read from the start.
///       Programs run one at a time, so the writer must finish before the reader starts.
///@param EAX: Pipe syscall num (24), on return the file descriptor or an error
static force_inline int pipe_syscall(void)
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(24)
        : "memory"
    );

    return ret;
}

///@brief Select the keyboard layout and/or get the current one
///@param EAX: Keymap syscall num (25), on return the status (FILE_NOT_FOUND for an unknown layout,
///            FILE_INVALID if the name doesn't fit in size)
///@param EBX: Layout to select ("it", "us"), NULL to keep the current one
///@param ECX: Buffer for the name of the current layout (nullable)
///@param EDX: Buffer size, terminator included
static force_inline int keymap_syscall(char *name, char *current, size_t size)
{
    int ret;

    __asm__ __volatile__ (
        "int $0x80 \n"
        : "=a"(ret)
        : "a"(25), "b"((uintptr_t)name), "c"((uintptr_t)current), "d"(size)
        : "memory"
    );

    return ret;
}

#endif
