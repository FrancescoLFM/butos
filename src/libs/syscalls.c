/* BUTOS SYSCALL COLLECTION */
/* NOT POSIX COMPLIANT (FOR NOW)*/

#include <drivers/vga.h>
#include <libs/print.h>
#include <libs/scan.h>
#include <libs/alloc.h>
#include <cpu/idt.h>
#include <cpu/proc.h>
#include <fs/fd.h>
#include <fs/blk.h>
#include <include/version.h>
#include <libs/string.h>
#include <include/asm.h>
#include <drivers/console.h>
#include <drivers/keyboard.h>
#include <stdarg.h>

/* Output of printf_handler when stdout is redirected, written in chunks */
struct stdout_buffer {
    char data[64];
    size_t len;
};

static void stdout_flush(struct stdout_buffer *out)
{
    fd_write(FD_STDOUT, out->data, out->len);
    out->len = 0;
}

static void stdout_out(void *ctx, char c)
{
    struct stdout_buffer *out = ctx;

    out->data[out->len++] = c;
    if (out->len == sizeof(out->data))
        stdout_flush(out);
}

/* The print syscalls write to the screen unless the process stdout is redirected */
static int stdout_redirected(void)
{
    return fd_console_stream(FD_STDOUT) != FD_STDOUT;
}

void putc_handler(struct registers_t *regs)
{
    char c = (char) regs->ebx;

    if (stdout_redirected())
        fd_write(FD_STDOUT, &c, 1);
    else
        putc(c);
}

void puts_handler(struct registers_t *regs)
{
    char *str = (char *) regs->ebx;

    if (stdout_redirected())
        fd_write(FD_STDOUT, str, strlen(str));
    else
        puts(str);
}

void printf_handler(struct registers_t *regs)
{
    char *str = (char *) regs->ebx;
    va_list *list_ptr = (va_list *) regs->ecx;
    struct stdout_buffer out = {.len = 0};

    if (!stdout_redirected()) {
        vprintk_c(STD_COLOR, str, *list_ptr);
        return;
    }
    vprintk_out(stdout_out, &out, str, *list_ptr);
    stdout_flush(&out);
}

void clear_handler(struct registers_t *regs)
{
    vga_clear((uint8_t) regs->ebx);
}

void getchar_handler(struct registers_t *regs)
{
    char *output = (char *) regs->ebx;

    if (fd_console_stream(FD_STDIN) == FD_STDIN) {
        *output = (char) getchar();
        return;
    }
    /* Redirected stdin: the end of the input reads as Ctrl+D */
    if (fd_read(FD_STDIN, output, 1) != 1)
        *output = CONSOLE_EOF;
}

void kalloc_handler(struct registers_t *regs)
{
    void **buff_ptr = (void **) regs->ebx;
    size_t buff_size = regs->ecx;
    
    *buff_ptr = kalloc(buff_size);
}

void kfree_handler(struct registers_t *regs)
{
    kfree((void *) regs->ebx);
}

void krealloc_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) krealloc((void *) regs->ebx, regs->ecx);
}

void exec_handler(struct registers_t *regs)
{
    char *path = (char *) regs->ebx;
    int *exit_code_ptr = (int *) regs->ecx;
    char **argv = (char **) regs->edx;
    int *stdio = (int *) regs->esi;
    int exit_code = 0;

    /* The caller's image is unmapped while the child runs, write back only after */
    regs->eax = (uint32_t) process_spawn(path, argv, stdio, &exit_code);
    if (exit_code_ptr != NULL)
        *exit_code_ptr = exit_code;
}

void puts_color_handler(struct registers_t *regs)
{
    char *str = (char *) regs->ecx;

    if (stdout_redirected())
        fd_write(FD_STDOUT, str, strlen(str));
    else
        puts_c((uint8_t) regs->ebx, str);
}

void open_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_open((char *) regs->ebx);
}

void close_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_close((int) regs->ebx);
}

void read_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_read((int) regs->ebx, (void *) regs->ecx, (size_t) regs->edx);
}

void write_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_write((int) regs->ebx, (void *) regs->ecx, (size_t) regs->edx);
}

void seek_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_seek((int) regs->ebx, (int) regs->ecx, (int) regs->edx);
}

void create_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_create((char *) regs->ebx);
}

void remove_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_remove((char *) regs->ebx);
}

void readdir_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_readdir((char *) regs->ebx, (int) regs->ecx, (fd_dirent_t *) regs->edx);
}

void chdir_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_chdir((char *) regs->ebx);
}

void getcwd_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_getcwd((char *) regs->ebx, (size_t) regs->ecx);
}

void mkdir_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_mkdir((char *) regs->ebx);
}

void rmdir_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_rmdir((char *) regs->ebx);
}

void uname_handler(struct registers_t *regs)
{
    struct utsname *name = (struct utsname *) regs->ebx;

    if (name == NULL) {
        regs->eax = (uint32_t) FD_INVALID;
        return;
    }
    memset(name, 0, sizeof(*name));
    strcpy(name->sysname, BUTOS_SYSNAME);
    strcpy(name->nodename, BUTOS_NODENAME);
    strcpy(name->release, BUTOS_RELEASE);
    /* Build time of this file */
    strcpy(name->version, __DATE__ " " __TIME__);
    strcpy(name->machine, BUTOS_MACHINE);
    regs->eax = FD_OK;
}

void blkinfo_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) blk_info((int) regs->ebx, (blk_info_t *) regs->ecx);
}

void pipe_handler(struct registers_t *regs)
{
    regs->eax = (uint32_t) fd_pipe();
}

void keymap_handler(struct registers_t *regs)
{
    char *name = (char *) regs->ebx;
    char *current = (char *) regs->ecx;
    size_t size = (size_t) regs->edx;
    const char *layout;

    if (name != NULL && keyboard_set_layout(name)) {
        regs->eax = (uint32_t) FD_NOT_FOUND;
        return;
    }
    layout = keyboard_layout();
    if (current != NULL) {
        if (strlen(layout) + 1 > size) {
            regs->eax = (uint32_t) FD_INVALID;
            return;
        }
        strcpy(current, layout);
    }
    regs->eax = FD_OK;
}
