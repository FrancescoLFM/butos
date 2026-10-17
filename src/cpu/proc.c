#include <cpu/proc.h>
#include <include/asm.h>
#include <libs/allocator.h>
#include <libs/alloc.h>
#include <libs/print.h>
#include <libs/scan.h>
#include <cpu/paging.h>
#include <cpu/vmm.h>
#include <libs/string.h>
#include <fs/fd.h>

/* Physical memory reserved to process images */
#define PROC_PMEM_START     0x01000000
#define PROC_PMEM_SIZE      0x0F000000
#define INIT_REGISTRY_SIZE  (200 * sizeof(struct memspace))
/* Processes can't be mapped over the identity mapped area or the kernel */
#define PROC_VMEM_START     INDEX_TO_ADDR
#define PROC_VMEM_END       KERNEL_VIRTUAL_START

static struct allocator proc_allocator;
static int proc_initialized = 0;
/* Filesystem programs are loaded from by process_spawn */
static fat_fs_t *proc_root_fs;
/* Number of processes currently running, nested through exec */
static int proc_depth = 0;
/* Live processes, indexed by depth - 1: the last one is the running process */
static struct process proc_table[PROC_MAX];
static int proc_next_pid = 1;

static force_inline uintptr_t get_cr3()
{
    uintptr_t cr3;

    __asm__ __volatile__ ("mov %%cr3, %0" : "=r"(cr3));

    return cr3;
}

int process_init(fat_fs_t *root_fs) 
{
    void *registry;

    if (proc_initialized)
        return EXIT_SUCCESS;

    registry = kalloc(INIT_REGISTRY_SIZE);
    if (registry == NULL)
        return EXIT_FAILURE;
    if (allocator_init(&proc_allocator, PROC_PMEM_START, PROC_PMEM_SIZE, registry, INIT_REGISTRY_SIZE, PAGE_FRAME_SIZE)) {
        kfree(registry);
        return EXIT_FAILURE;
    }
    proc_root_fs = root_fs;
    proc_initialized = 1;

    return EXIT_SUCCESS;
}

/* Page aligned virtual span covering every loadable segment */
static int process_image_span(elf_t *elf, uintptr_t *start, uintptr_t *end)
{
    struct elf_p_header *p_header;
    uintptr_t seg_end;

    *start = UINTPTR_MAX;
    *end = 0;
    for (u16 i = 0; i < elf->header.p_entry_num; i++) {
        p_header = &elf->p_headers[i];
        if (p_header->segment_type != PT_LOAD || p_header->p_memsz == 0)
            continue;
        seg_end = p_header->p_vaddr + p_header->p_memsz;
        if (seg_end < p_header->p_vaddr || p_header->p_filez > p_header->p_memsz)
            return EXIT_FAILURE;
        if (p_header->p_vaddr < *start)
            *start = p_header->p_vaddr;
        if (seg_end > *end)
            *end = seg_end;
    }
    if (*start >= *end)
        return EXIT_FAILURE;

    *start = PAGE_ROUND_DOWN(*start);
    *end = PAGE_ROUND_UP(*end);
    if (*start < PROC_VMEM_START || *end > PROC_VMEM_END || *end == 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

/* The program runs in ring 0 on the kernel stack and follows the cdecl ABI: main(argc, argv) */
static int process_call(uintptr_t entry, int argc, char **argv)
{
    int ret;

    __asm__ __volatile__ (
        "push %3 \n"
        "push %2 \n"
        "call *%1 \n"
        "add $8, %%esp"
        : "=a"(ret)
        : "r"(entry), "r"(argc), "r"(argv)
        : "ecx", "edx", "memory", "cc"
    );

    return ret;
}

/* Registers the process about to run as the innermost one, the caller must check PROC_MAX */
static void process_table_push(int argc, char **argv, const int *stdio, uintptr_t entry,
                               uintptr_t vstart, uintptr_t vend, uintptr_t paddr)
{
    struct process *parent = process_current();
    struct process *proc = &proc_table[proc_depth];
    const char *name = (argc > 0 && argv[0] != NULL) ? argv[0] : "?";
    const char *slash = strrchr(name, '/');

    if (slash != NULL && slash[1] != '\0')
        name = slash + 1;
    memset(proc, 0, sizeof(*proc));
    proc->pid = proc_next_pid++;
    proc->ppid = parent != NULL ? parent->pid : 0;
    proc->depth = proc_depth + 1;
    proc->state = PROC_STATE_RUNNING;
    strncpy(proc->name, name, PROC_NAME_LEN - 1);
    proc->argc = argc;
    proc->argv = argv;
    proc->entry = entry;
    proc->vstart = vstart;
    proc->vend = vend;
    proc->paddr = paddr;
    strcpy(proc->cwd, process_cwd());
    for (int i = 0; i < FD_STD_NUM; i++)
        proc->stdio[i] = stdio != NULL ? stdio[i] : (parent != NULL ? parent->stdio[i] : i);
    if (parent != NULL)
        parent->state = PROC_STATE_WAITING;
    proc_depth++;
}

static void process_table_pop(void)
{
    struct process *parent;

    proc_depth--;
    memset(&proc_table[proc_depth], 0, sizeof(proc_table[proc_depth]));
    parent = process_current();
    if (parent != NULL)
        parent->state = PROC_STATE_RUNNING;
}

int process_exec(elf_t *elf, int argc, char **argv, const int *stdio, int *exit_code)
{
    uint32_t *kernel_dir;
    uint32_t *page_dir;
    uintptr_t parent_dir;
    uintptr_t vstart, vend, paddr;
    uintptr_t entry = elf->header.p_entry_offset;
    int status = EXIT_FAILURE;

    if (!proc_initialized || proc_depth == PROC_MAX || process_image_span(elf, &vstart, &vend))
        return EXIT_FAILURE;
    if (entry < vstart || entry >= vend)
        return EXIT_FAILURE;

    /* The whole image is physically contiguous, so segments sharing a page are fine */
    paddr = allocator_alloc(&proc_allocator, vend - vstart);
    if (paddr == 0)
        return EXIT_FAILURE;

    page_dir = aligned_kalloc(PAGE_FRAME_SIZE, PAGE_DIR_SIZE * sizeof(*page_dir));
    if (page_dir == NULL)
        goto free_image;
    kernel_dir = get_blank_page_directory();
    /* A process can exec another one (e.g. the shell), its mappings must come back after */
    parent_dir = get_cr3();
    memcpy(page_dir, kernel_dir, PAGE_DIR_SIZE * sizeof(*page_dir));
    if (paging_map_range(page_dir, kernel_dir, vstart, paddr, vend - vstart, KERNEL_PAGE_ATTR))
        goto free_dir;

    page_directory_load((uint32_t *) kernel_virtual_to_physical((uintptr_t) page_dir));
    for (u16 i = 0; i < elf->header.p_entry_num; i++) {
        if (elf->p_headers[i].segment_type == PT_LOAD && elf->p_headers[i].p_memsz)
            if (p_header_memload(&elf->p_headers[i], elf))
                goto restore_dir;
    }
    process_table_push(argc, argv, stdio, entry, vstart, vend, paddr);
    *exit_code = process_call(entry, argc, argv);
    /* Files the process left open */
    fd_close_owned(proc_depth);
    process_table_pop();
    status = EXIT_SUCCESS;

restore_dir:
    page_directory_load((uint32_t *) parent_dir);
free_dir:
    paging_free_tables(page_dir, kernel_dir);
    kfree(page_dir);
free_image:
    allocator_free(&proc_allocator, paddr);

    return status;
}

int process_depth(void)
{
    return proc_depth;
}

struct process *process_at(int depth)
{
    if (depth < 1 || depth > proc_depth)
        return NULL;

    return &proc_table[depth - 1];
}

struct process *process_current(void)
{
    return process_at(proc_depth);
}

const char *process_cwd(void)
{
    struct process *proc = process_current();

    return proc != NULL ? proc->cwd : PROC_ROOT_CWD;
}

struct process *process_get(int pid)
{
    for (int i = 0; i < proc_depth; i++)
        if (proc_table[i].pid == pid)
            return &proc_table[i];

    return NULL;
}

/* 
    Copies argv (NULL terminated, NULL means just path) in one kernel heap block:
    the pointer array followed by the strings. The caller's image is unmapped
    while the new process runs, so its strings can't be passed as they are.
*/
static char **process_args_copy(char *path, char **argv, int *argc)
{
    char *default_argv[] = {path, NULL};
    char **copy;
    char *str;
    size_t size = 0, len;
    int count = 0;

    if (argv == NULL)
        argv = default_argv;
    for (; argv[count] != NULL; count++) {
        if (count == PROC_MAX_ARGS)
            return NULL;
        size += strlen(argv[count]) + 1;
        if (size > PROC_MAX_ARGS_SIZE)
            return NULL;
    }

    copy = kalloc((count + 1) * sizeof(char *) + size);
    if (copy == NULL)
        return NULL;
    str = (char *) &copy[count + 1];
    for (int i = 0; i < count; i++) {
        len = strlen(argv[i]) + 1;
        memcpy(str, argv[i], len);
        copy[i] = str;
        str += len;
    }
    copy[count] = NULL;
    *argc = count;

    return copy;
}

proc_status_t process_spawn(char *path, char **argv, const int *stdio, int *exit_code)
{
    elf_t elf;
    file_t *file;
    char resolved[PATH_MAX];
    char **args;
    int argc;
    /* Resolved while the caller is still the running process */
    int targets[FD_STD_NUM];
    proc_status_t status = PROC_OK;

    if (!proc_initialized || proc_root_fs == NULL)
        return PROC_EXEC_FAILED;
    for (int i = 0; stdio != NULL && i < FD_STD_NUM; i++)
        if ((targets[i] = fd_target(stdio[i])) < 0)
            return PROC_INVALID_ARGS;
    /* path may live in the caller's image: it's only read before switching page directory */
    if (path_resolve(process_cwd(), path, resolved, sizeof(resolved)))
        return PROC_NOT_FOUND;
    file = file_open_path(proc_root_fs, resolved);
    if (file == NULL)
        return PROC_NOT_FOUND;
    args = process_args_copy(path, argv, &argc);
    if (args == NULL) {
        file_close(proc_root_fs, file);
        return PROC_INVALID_ARGS;
    }
    if (elf_init(&elf, file, proc_root_fs))
        status = PROC_INVALID_EXEC;
    else if (process_exec(&elf, argc, args, stdio != NULL ? targets : NULL, exit_code))
        status = PROC_EXEC_FAILED;

    elf_fini(&elf);
    file_close(proc_root_fs, file);
    kfree(args);

    return status;
}
