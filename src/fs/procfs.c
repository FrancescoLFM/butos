#include <fs/procfs.h>
#include <cpu/proc.h>
#include <libs/alloc.h>
#include <libs/print.h>
#include <libs/string.h>

/* Generated files are truncated to this size */
#define PROCFS_FILE_MAX     512
/* <pid>/fd/<fd> is the deepest path */
#define PROCFS_PARTS_MAX    3
/* "/proc/<pid>/cmdline" */
#define PROCFS_PATH_MAX     32

/* Components of a path below /proc, pointing into the original string */
struct procfs_path {
    int count;
    const char *part[PROCFS_PARTS_MAX];
    size_t len[PROCFS_PARTS_MAX];
};

struct procfs_text {
    char *buf;
    size_t len;
};

typedef enum {
    PROCFS_STATUS,
    PROCFS_CMDLINE,
    PROCFS_FD_DIR,
} procfs_entry_t;

/* Content of /proc/<pid>, in readdir order */
static const struct {
    const char *name;
    procfs_entry_t type;
} procfs_pid_entries[] = {
    {"status", PROCFS_STATUS},
    {"cmdline", PROCFS_CMDLINE},
    {"fd", PROCFS_FD_DIR},
};

int procfs_match(const char *path)
{
    size_t len = strlen(PROCFS_ROOT);

    return strncmp(path, PROCFS_ROOT, len) == 0 && (path[len] == '\0' || path[len] == '/');
}

static int procfs_split(const char *path, struct procfs_path *out)
{
    path += strlen(PROCFS_ROOT);
    out->count = 0;
    for (;;) {
        while (*path == '/')
            path++;
        if (*path == '\0')
            return EXIT_SUCCESS;
        if (out->count == PROCFS_PARTS_MAX)
            return EXIT_FAILURE;
        out->part[out->count] = path;
        while (*path && *path != '/')
            path++;
        out->len[out->count] = (size_t) (path - out->part[out->count]);
        out->count++;
    }
}

static int procfs_part_is(struct procfs_path *path, int i, const char *name)
{
    return strlen(name) == path->len[i] && strncmp(path->part[i], name, path->len[i]) == 0;
}

/* -1 if the component isn't a decimal number */
static int procfs_part_number(struct procfs_path *path, int i)
{
    int num = 0;

    if (path->len[i] == 0 || path->len[i] > 9)
        return -1;
    for (size_t j = 0; j < path->len[i]; j++) {
        if (path->part[i][j] < '0' || path->part[i][j] > '9')
            return -1;
        num = num * 10 + path->part[i][j] - '0';
    }

    return num;
}

/* Process named by the first component, "self" or a pid */
static struct process *procfs_process(struct procfs_path *path)
{
    int pid;

    if (procfs_part_is(path, 0, "self"))
        return process_current();
    pid = procfs_part_number(path, 0);

    return pid < 0 ? NULL : process_get(pid);
}

/* Entry of /proc/<pid> named by the second component, -1 if there is none */
static int procfs_pid_entry(struct procfs_path *path)
{
    for (size_t i = 0; i < ARR_SIZE(procfs_pid_entries); i++)
        if (procfs_part_is(path, 1, procfs_pid_entries[i].name))
            return (int) procfs_pid_entries[i].type;

    return -1;
}

/* The standard streams are shared, the other descriptors belong to the process that opened them */
static int procfs_fd_visible(struct process *proc, int fd)
{
    int owner = fd_owner(fd);

    return owner >= 0 && (fd < FD_STD_NUM || owner == proc->depth);
}

static void text_puts(struct procfs_text *text, const char *str)
{
    while (*str && text->len < PROCFS_FILE_MAX)
        text->buf[text->len++] = *str++;
}

static void text_num(struct procfs_text *text, uint32_t num, uint32_t base)
{
    char digits[33];

    if (base == 16)
        text_puts(text, "0x");
    if (base_convert(digits, sizeof(digits), num, base) > 0)
        text_puts(text, digits);
}

static void text_field(struct procfs_text *text, const char *name, uint32_t num, uint32_t base)
{
    text_puts(text, name);
    text_num(text, num, base);
    text_puts(text, "\n");
}

static void procfs_status(struct process *proc, struct procfs_text *text)
{
    int fds = 0;

    for (int fd = 0; fd < FD_MAX; fd++)
        fds += procfs_fd_visible(proc, fd);

    text_puts(text, "Name:   ");
    text_puts(text, proc->name);
    text_puts(text, "\n");
    text_field(text, "Pid:    ", (uint32_t) proc->pid, 10);
    text_field(text, "PPid:   ", (uint32_t) proc->ppid, 10);
    text_puts(text, proc->state == PROC_STATE_RUNNING ? "State:  R (running)\n" : "State:  S (waiting)\n");
    text_field(text, "Depth:  ", (uint32_t) proc->depth, 10);
    text_puts(text, "Cwd:    ");
    text_puts(text, proc->cwd);
    text_puts(text, "\n");
    text_field(text, "Entry:  ", proc->entry, 16);
    text_puts(text, "Image:  ");
    text_num(text, proc->vstart, 16);
    text_puts(text, "-");
    text_num(text, proc->vend, 16);
    text_puts(text, "\n");
    text_field(text, "Phys:   ", proc->paddr, 16);
    text_field(text, "Size:   ", proc->vend - proc->vstart, 10);
    text_field(text, "FDs:    ", (uint32_t) fds, 10);
}

static void procfs_cmdline(struct process *proc, struct procfs_text *text)
{
    for (int i = 0; i < proc->argc; i++) {
        if (i > 0)
            text_puts(text, " ");
        text_puts(text, proc->argv[i]);
    }
    text_puts(text, "\n");
}

int procfs_open(const char *path)
{
    struct procfs_path parts;
    struct procfs_text text;
    struct process *proc;
    char canonical[PROCFS_PATH_MAX];
    char pid[12];
    int entry, fd;

    if (procfs_split(path, &parts))
        return FD_NOT_FOUND;
    /* /proc and /proc/<pid> are directories */
    if (parts.count < 2)
        return parts.count == 0 || procfs_process(&parts) != NULL ? FD_INVALID : FD_NOT_FOUND;
    proc = procfs_process(&parts);
    entry = procfs_pid_entry(&parts);
    if (proc == NULL || entry < 0)
        return FD_NOT_FOUND;

    if (entry == PROCFS_FD_DIR) {
        if (parts.count == 2)
            return FD_INVALID;
        fd = procfs_part_number(&parts, 2);
        if (parts.count != 3 || fd < 0 || fd >= FD_MAX || !procfs_fd_visible(proc, fd))
            return FD_NOT_FOUND;
        return fd_reopen(fd);
    }
    if (parts.count != 2)
        return FD_NOT_FOUND;

    text.buf = kalloc(PROCFS_FILE_MAX);
    if (text.buf == NULL)
        return FD_IO_ERROR;
    text.len = 0;
    if (entry == PROCFS_STATUS)
        procfs_status(proc, &text);
    else
        procfs_cmdline(proc, &text);

    /* Saved with the pid instead of "self", it's the path /proc/<pid>/fd reopens */
    base_convert(pid, sizeof(pid), (uint32_t) proc->pid, 10);
    strcpy(canonical, PROCFS_ROOT "/");
    strcat(canonical, pid);
    strcat(canonical, "/");
    strncat(canonical, parts.part[1], parts.len[1]);

    return fd_open_buffer(canonical, text.buf, text.len);
}

static void procfs_dirent_num(fd_dirent_t *out, int num, int is_dir)
{
    base_convert(out->name, sizeof(out->name), (uint32_t) num, 10);
    out->is_dir = is_dir;
    out->size = 0;
}

static void procfs_dirent_name(fd_dirent_t *out, const char *name, int is_dir)
{
    strncpy(out->name, name, sizeof(out->name) - 1);
    out->name[sizeof(out->name) - 1] = '\0';
    out->is_dir = is_dir;
    out->size = 0;
}

/* "self" (when called by a process) followed by every live pid */
static int procfs_readdir_root(int index, fd_dirent_t *out)
{
    struct process *proc;

    if (process_current() != NULL) {
        if (index == 0) {
            procfs_dirent_name(out, "self", 1);
            return 1;
        }
        index--;
    }
    proc = process_at(index + 1);
    if (proc == NULL)
        return 0;
    procfs_dirent_num(out, proc->pid, 1);

    return 1;
}

static int procfs_readdir_fd(struct process *proc, int index, fd_dirent_t *out)
{
    for (int fd = 0; fd < FD_MAX; fd++) {
        if (!procfs_fd_visible(proc, fd))
            continue;
        if (index-- == 0) {
            procfs_dirent_num(out, fd, 0);
            return 1;
        }
    }

    return 0;
}

int procfs_readdir(const char *path, int index, fd_dirent_t *out)
{
    struct procfs_path parts;
    struct process *proc;
    int entry;

    if (procfs_split(path, &parts))
        return FD_NOT_FOUND;
    if (parts.count == 0)
        return procfs_readdir_root(index, out);
    proc = procfs_process(&parts);
    if (proc == NULL)
        return FD_NOT_FOUND;

    if (parts.count == 1) {
        if ((size_t) index >= ARR_SIZE(procfs_pid_entries))
            return 0;
        procfs_dirent_name(out, procfs_pid_entries[index].name,
                           procfs_pid_entries[index].type == PROCFS_FD_DIR);
        return 1;
    }

    entry = procfs_pid_entry(&parts);
    if (entry < 0 || parts.count > 2)
        return FD_NOT_FOUND;
    if (entry != PROCFS_FD_DIR)
        return FD_INVALID;

    return procfs_readdir_fd(proc, index, out);
}
