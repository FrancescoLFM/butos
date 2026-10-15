#ifndef PROC_H
#define PROC_H

#include <fs/elf.h>
#include <fs/path.h>

#define DEFAULT_STACK   1024
#define DEFAULT_HEAP    1000000
/* Limits on the arguments passed to process_spawn */
#define PROC_MAX_ARGS       32
#define PROC_MAX_ARGS_SIZE  1024
/* Processes nest through exec on the kernel stack, this bounds the nesting depth */
#define PROC_MAX            16
#define PROC_NAME_LEN       16
/* Working directory of the processes started by the kernel */
#define PROC_ROOT_CWD       "/"

typedef enum {
    /* The innermost process, the one executing */
    PROC_STATE_RUNNING,
    /* Blocked in exec until its child exits */
    PROC_STATE_WAITING,
} proc_state_t;

/* Entry of the process table, exposed under /proc/<pid> */
struct process {
    int pid;
    /* 0 when started by the kernel */
    int ppid;
    /* Nesting depth, starts from 1: owner of the descriptors the process opens */
    int depth;
    proc_state_t state;
    /* Basename of argv[0] */
    char name[PROC_NAME_LEN];
    int argc;
    /* Kernel memory, valid while the process runs */
    char **argv;
    uintptr_t entry;
    /* Page aligned virtual span of the image and its physical base */
    uintptr_t vstart;
    uintptr_t vend;
    uintptr_t paddr;
    /* Normalized absolute path, inherited from the parent: relative paths start from here */
    char cwd[PATH_MAX];
};

typedef enum {
    PROC_OK = 0,
    PROC_NOT_FOUND,
    PROC_INVALID_EXEC,
    PROC_EXEC_FAILED,
    PROC_INVALID_ARGS,
} proc_status_t;

int process_init(fat_fs_t *root_fs);
/* Runs the loaded elf as main(argc, argv), argv must be in kernel memory */
int process_exec(elf_t *elf, int argc, char **argv, int *exit_code);
/* Loads the executable at path from the root filesystem and runs it to completion,
   argv is NULL terminated with the program name first (NULL means just path) */
proc_status_t process_spawn(char *path, char **argv, int *exit_code);
/* Nesting depth of the running process, 0 when called from the kernel */
int process_depth(void);
/* Process at nesting depth (1 to process_depth()), NULL if there is none */
struct process *process_at(int depth);
/* The running process, NULL when called from the kernel */
struct process *process_current(void);
/* NULL if no live process has that pid */
struct process *process_get(int pid);
/* Working directory of the running process, PROC_ROOT_CWD when called from the kernel */
const char *process_cwd(void);

#endif