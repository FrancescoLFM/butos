# VFS Shim + Unix Directory Layout — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Introduce virtual (RAM-backed) files accessible via the existing file syscalls, and reorganise the FAT32 partition into a Unix-style directory tree with programs in `/usr/bin`.

**Architecture:** A static `vfd_table[]` in `fd.c` holds registered RAM-backed paths. `fd_open` checks that table first; if matched, an `fd_entry` of type `FD_TYPE_RAM` is created and all operations dispatch on `type`. Directory layout (`/usr/bin`, `/bin`, `/etc`, `/tmp`) is created at build time via `mmd`; butosh searches those dirs in order.

**Tech Stack:** C99 freestanding, GNU as, mtools (`mkfs.fat`, `mmd`, `mcopy`), QEMU.

**Spec:** `docs/superpowers/specs/2026-10-14-vfs-shim-unix-dirs-design.md`

## Global Constraints

- All C compiled with `-Wall -Wextra -Werror -m32 -ffreestanding -nostdlib -std=c99` — warnings are errors.
- `-mgeneral-regs-only`: no FP/SSE.
- No test framework; in-kernel functions called from `main()` are the test harness.
- POSIX shell only in makefile recipes (no `echo -e`, GNU `stat`, `sed -i` without suffix).
- `$(CC)`, `$(AS)`, `$(LD)`, `$(MAKE)` — never bare tool names in makefiles.
- 8.3 names on FAT32 for real files (virtual files are exempt).
- Includes rooted at `src/` (`#include <fs/fd.h>`).

## Review Focus

- **`vfd_register` before `fd_init`**: vfd_table is a static global (zero-init); `fd_open` must check vfd_table *before* the `fd_root_fs == NULL` guard. Covered in Task 2 test.
- **Write past end of RAM buffer**: `pos >= ram.size` must return `FD_IO_ERROR`, not corrupt memory. Covered in Task 2 test.
- **Seek past end**: `pos > ram.size` must return `FD_INVALID`. Covered in Task 2 test.
- **`vfd_unregister` with open fd**: must return `FD_BUSY`. Covered in Task 2 test.
- **Programs longer than `PATH_SIZE - 9`**: building `/usr/bin/<name>` overflows path buffer if `PATH_SIZE` stays at 130. Fixed in Task 6 by widening to `BUFFER_SIZE + 16`.

---

## Task 1: fd.h — new types and API declarations

**Files:**
- Modify: `src/fs/fd.h`

**Interfaces:**
- Produces: `fd_type_t`, updated `fd_entry`, `VFD_MAX`, `VFD_PATH_MAX`, `vfd_register`, `vfd_unregister`

- [ ] **Step 1: Add constants and `fd_type_t` above `FD_MAX`**

In `src/fs/fd.h`, after the existing `#include` lines and before `#define FD_MAX`, insert:

```c
#define VFD_MAX       8
#define VFD_PATH_MAX  64

typedef enum {
    FD_TYPE_FAT32,
    FD_TYPE_RAM,
} fd_type_t;
```

- [ ] **Step 2: Replace the existing `struct fd_entry` forward declaration (if any) — it lives in `fd.c`, not the header — just verify `fd.h` has no struct definition to change**

`fd_entry` is defined in `fd.c`; the header only declares function prototypes. No struct change needed in the header.

- [ ] **Step 3: Add `vfd_register` and `vfd_unregister` prototypes after `fd_close_owned`**

```c
/* Register/unregister RAM-backed virtual files.
 * vfd_register may be called before or after fd_init.
 * vfd_unregister fails with FD_BUSY if the file is open. */
fd_status_t vfd_register(const char *path, void *buf, size_t size, int writable);
fd_status_t vfd_unregister(const char *path);
```

- [ ] **Step 4: Compile-check**

```bash
make -C /home/gian/Documenti/Progetti/butos/src
```

Expected: clean build (no new warnings, no errors).

- [ ] **Step 5: Commit**

```bash
git add src/fs/fd.h
git commit -m "fs/fd: add VFD_MAX, fd_type_t, vfd_register/unregister declarations"
```

---

## Task 2: fd.c — VFD table and full dispatch

**Files:**
- Modify: `src/fs/fd.c`

**Interfaces:**
- Consumes: `fd_type_t`, `VFD_MAX`, `VFD_PATH_MAX`, `vfd_register`, `vfd_unregister` from Task 1
- Produces: working `vfd_register`, `vfd_unregister`, `fd_open` that resolves virtual paths, and all operations dispatching correctly on `FD_TYPE_RAM`

- [ ] **Step 1: Add `vfd_entry` struct and `vfd_table` static array after the existing `fd_entry` struct**

Replace the existing `struct fd_entry` definition (currently at the top of `fd.c`) with:

```c
struct vfd_entry {
    int used;
    char path[VFD_PATH_MAX];
    uint8_t *buf;
    size_t size;
    int writable;
};

struct fd_entry {
    int used;
    int dirty;
    int owner;
    uint32_t pos;
    fd_type_t type;
    union {
        file_t *file;
        struct {
            uint8_t *buf;
            size_t   size;
            int      writable;
        } ram;
    };
};

static fat_fs_t *fd_root_fs;
static struct fd_entry fd_table[FD_MAX];
static struct vfd_entry vfd_table[VFD_MAX];
```

Remove the old `static fat_fs_t *fd_root_fs;` and `static struct fd_entry fd_table[FD_MAX];` lines (they are now in the block above).

- [ ] **Step 2: Update `fd_init` — zero only `fd_table`, not `vfd_table` (it may have been populated before init)**

```c
void fd_init(fat_fs_t *root_fs)
{
    fd_root_fs = root_fs;
    memset(fd_table, 0, sizeof(fd_table));
}
```

(No change from today — the `memset` already only covers `fd_table`. Confirm `vfd_table` is not zeroed here; it is a static global so it starts zero-initialised by the C runtime.)

- [ ] **Step 3: Add `vfd_register` and `vfd_unregister` implementations before `fd_open`**

```c
fd_status_t vfd_register(const char *path, void *buf, size_t size, int writable)
{
    int i;

    if (path == NULL || *path != '/' || buf == NULL)
        return FD_INVALID;
    if (strlen(path) >= VFD_PATH_MAX)
        return FD_INVALID;

    for (i = 0; i < VFD_MAX; i++)
        if (vfd_table[i].used && strcmp(vfd_table[i].path, path) == 0)
            return FD_EXISTS;

    for (i = 0; i < VFD_MAX; i++) {
        if (!vfd_table[i].used) {
            strncpy(vfd_table[i].path, path, VFD_PATH_MAX - 1);
            vfd_table[i].path[VFD_PATH_MAX - 1] = '\0';
            vfd_table[i].buf      = (uint8_t *) buf;
            vfd_table[i].size     = size;
            vfd_table[i].writable = writable;
            vfd_table[i].used     = 1;
            return FD_OK;
        }
    }
    return FD_TOO_MANY;
}

fd_status_t vfd_unregister(const char *path)
{
    int i, fd;

    if (path == NULL)
        return FD_INVALID;

    for (i = 0; i < VFD_MAX; i++) {
        if (!vfd_table[i].used || strcmp(vfd_table[i].path, path) != 0)
            continue;
        /* Refuse if any open fd points at this registration */
        for (fd = 0; fd < FD_MAX; fd++) {
            if (fd_table[fd].used && fd_table[fd].type == FD_TYPE_RAM &&
                fd_table[fd].ram.buf == vfd_table[i].buf)
                return FD_BUSY;
        }
        vfd_table[i].used = 0;
        return FD_OK;
    }
    return FD_NOT_FOUND;
}
```

- [ ] **Step 4: Rewrite `fd_open` to check vfd_table first**

```c
int fd_open(char *path)
{
    int fd, i;

    if (path == NULL || *path != '/')
        return FD_INVALID;

    for (fd = 0; fd < FD_MAX && fd_table[fd].used; fd++);
    if (fd == FD_MAX)
        return FD_TOO_MANY;

    /* Virtual files take priority over FAT32 */
    for (i = 0; i < VFD_MAX; i++) {
        if (!vfd_table[i].used || strcmp(vfd_table[i].path, path) != 0)
            continue;
        fd_table[fd].type        = FD_TYPE_RAM;
        fd_table[fd].ram.buf     = vfd_table[i].buf;
        fd_table[fd].ram.size    = vfd_table[i].size;
        fd_table[fd].ram.writable = vfd_table[i].writable;
        fd_table[fd].used        = 1;
        fd_table[fd].dirty       = 0;
        fd_table[fd].owner       = process_depth();
        fd_table[fd].pos         = 0;
        return fd;
    }

    if (fd_root_fs == NULL)
        return FD_INVALID;

    fd_table[fd].file = file_open_path(fd_root_fs, path);
    if (fd_table[fd].file == NULL)
        return FD_NOT_FOUND;
    fd_table[fd].type  = FD_TYPE_FAT32;
    fd_table[fd].used  = 1;
    fd_table[fd].dirty = 0;
    fd_table[fd].owner = process_depth();
    fd_table[fd].pos   = 0;
    return fd;
}
```

- [ ] **Step 5: Update `fd_close` to dispatch on type**

```c
fd_status_t fd_close(int fd)
{
    struct fd_entry *entry;
    fd_status_t status = FD_OK;

    entry = fd_get(fd);
    if (entry == NULL)
        return FD_INVALID;

    if (entry->type == FD_TYPE_FAT32) {
        if (entry->dirty && file_sync(fd_root_fs, entry->file))
            status = FD_IO_ERROR;
        file_close(fd_root_fs, entry->file);
        entry->file = NULL;
    }
    /* FD_TYPE_RAM: no sync, no free (buffer lifetime managed by caller) */
    entry->used = 0;
    return status;
}
```

- [ ] **Step 6: Update `fd_read` to dispatch on type**

```c
int fd_read(int fd, void *buffer, size_t size)
{
    struct fd_entry *entry;
    uint8_t *out = buffer;
    size_t avail, to_read, i;

    entry = fd_get(fd);
    if (entry == NULL || buffer == NULL)
        return FD_INVALID;

    if (entry->type == FD_TYPE_RAM) {
        avail   = (entry->pos < entry->ram.size) ? entry->ram.size - entry->pos : 0;
        to_read = (size < avail) ? size : avail;
        memcpy(out, entry->ram.buf + entry->pos, to_read);
        entry->pos += (uint32_t) to_read;
        return (int) to_read;
    }

    for (i = 0; i < size && entry->pos < entry->file->entry->size; i++, entry->pos++)
        out[i] = file_readb(entry->file, fd_root_fs, entry->pos);
    return (int) i;
}
```

- [ ] **Step 7: Update `fd_write` to dispatch on type**

```c
int fd_write(int fd, void *buffer, size_t size)
{
    struct fd_entry *entry;
    size_t written;

    entry = fd_get(fd);
    if (entry == NULL || buffer == NULL)
        return FD_INVALID;
    if (size == 0)
        return 0;

    if (entry->type == FD_TYPE_RAM) {
        if (!entry->ram.writable)
            return FD_IO_ERROR;
        if (entry->pos >= entry->ram.size)
            return FD_IO_ERROR;
        written = entry->ram.size - entry->pos;
        if (size < written)
            written = size;
        memcpy(entry->ram.buf + entry->pos, (uint8_t *) buffer, written);
        entry->pos += (uint32_t) written;
        return (int) written;
    }

    written = file_write(entry->file, fd_root_fs, entry->pos, buffer, size);
    entry->pos += written;
    if (written)
        entry->dirty = 1;
    if (written == 0)
        return FD_IO_ERROR;
    return (int) written;
}
```

- [ ] **Step 8: Update `fd_seek` to dispatch on type**

```c
int fd_seek(int fd, int offset, int whence)
{
    struct fd_entry *entry;
    uint32_t file_size;
    int base, pos;

    entry = fd_get(fd);
    if (entry == NULL)
        return FD_INVALID;

    file_size = (entry->type == FD_TYPE_FAT32)
        ? entry->file->entry->size
        : (uint32_t) entry->ram.size;

    switch (whence) {
    case FD_SEEK_SET: base = 0;                   break;
    case FD_SEEK_CUR: base = (int) entry->pos;    break;
    case FD_SEEK_END: base = (int) file_size;     break;
    default:          return FD_INVALID;
    }

    pos = base + offset;
    if (pos < 0 || (uint32_t) pos > file_size)
        return FD_INVALID;
    entry->pos = (uint32_t) pos;
    return pos;
}
```

- [ ] **Step 9: Guard `fd_create` and `fd_remove` against virtual paths**

In `fd_create`, add after the initial `NULL`/`'/'` check:

```c
    /* Virtual files cannot be created via syscall */
    for (int i = 0; i < VFD_MAX; i++)
        if (vfd_table[i].used && strcmp(vfd_table[i].path, path) == 0)
            return FD_EXISTS;
```

In `fd_remove`, add after the initial `NULL`/`'/'` check:

```c
    /* Virtual files cannot be removed via syscall */
    for (int i = 0; i < VFD_MAX; i++)
        if (vfd_table[i].used && strcmp(vfd_table[i].path, path) == 0)
            return FD_INVALID;
```

- [ ] **Step 10: Compile-check**

```bash
make -C /home/gian/Documenti/Progetti/butos/src
```

Expected: clean, no warnings.

- [ ] **Step 11: Commit**

```bash
git add src/fs/fd.c
git commit -m "fs/fd: add VFD shim — RAM-backed virtual files via fd_open/read/write/seek"
```

---

## Task 3: In-kernel VFD test

**Files:**
- Modify: `src/main.c` (temporary test function + call; reverted in Task 5)

**Interfaces:**
- Consumes: `vfd_register`, `vfd_unregister`, `fd_open`, `fd_read`, `fd_write`, `fd_seek`, `fd_close` from Task 2

- [ ] **Step 1: Add `vfd_test` static function in `src/main.c` before `init_start`**

Add `#include <fs/fd.h>` at the top if not already present (it is). Add before `init_start`:

```c
static void vfd_test(void)
{
    static char hello_buf[] = "Hello, VFD!";
    static char rw_buf[32];
    char read_buf[32];
    int fd, n;

    printk("=== vfd_test start ===\n");

    /* --- read-only registration and read --- */
    if (vfd_register("/dev/hello", hello_buf, 11, 0) != FD_OK) {
        printk("FAIL: vfd_register ro\n"); return;
    }
    fd = fd_open("/dev/hello");
    if (fd < 0) { printk("FAIL: open ro\n"); return; }
    n = fd_read(fd, read_buf, sizeof(read_buf));
    if (n != 11 || strncmp(read_buf, "Hello, VFD!", 11) != 0) {
        printk("FAIL: read content\n"); return;
    }
    /* EOF: next read returns 0 */
    if (fd_read(fd, read_buf, 1) != 0) { printk("FAIL: no EOF\n"); return; }
    /* seek back to 0, re-read one byte */
    if (fd_seek(fd, 0, FD_SEEK_SET) != 0) { printk("FAIL: seek set\n"); return; }
    if (fd_read(fd, read_buf, 1) != 1 || read_buf[0] != 'H') {
        printk("FAIL: seek+read\n"); return;
    }
    /* seek past end → FD_INVALID */
    if (fd_seek(fd, 12, FD_SEEK_SET) != FD_INVALID) { printk("FAIL: seek past end\n"); return; }
    /* write on read-only → FD_IO_ERROR */
    if (fd_write(fd, "x", 1) != FD_IO_ERROR) { printk("FAIL: write on ro\n"); return; }
    fd_close(fd);
    printk("PASS: read-only vfd\n");

    /* --- writable registration, write then read back --- */
    if (vfd_register("/dev/rw", rw_buf, sizeof(rw_buf), 1) != FD_OK) {
        printk("FAIL: vfd_register rw\n"); return;
    }
    fd = fd_open("/dev/rw");
    if (fd < 0) { printk("FAIL: open rw\n"); return; }
    n = fd_write(fd, "test_data", 9);
    if (n != 9) { printk("FAIL: write count\n"); return; }
    fd_seek(fd, 0, FD_SEEK_SET);
    memset(read_buf, 0, sizeof(read_buf));
    n = fd_read(fd, read_buf, 9);
    if (n != 9 || strncmp(read_buf, "test_data", 9) != 0) {
        printk("FAIL: rw readback\n"); return;
    }
    /* write past end → FD_IO_ERROR */
    fd_seek(fd, 0, FD_SEEK_END);
    if (fd_write(fd, "x", 1) != FD_IO_ERROR) { printk("FAIL: write past end\n"); return; }
    /* vfd_unregister with open fd → FD_BUSY */
    if (vfd_unregister("/dev/rw") != FD_BUSY) { printk("FAIL: busy check\n"); return; }
    fd_close(fd);
    printk("PASS: writable vfd\n");

    /* --- unregister then re-open fails --- */
    if (vfd_unregister("/dev/hello") != FD_OK) { printk("FAIL: unregister\n"); return; }
    if (fd_open("/dev/hello") >= 0) { printk("FAIL: open after unregister\n"); return; }
    printk("PASS: unregister\n");

    /* --- duplicate registration → FD_EXISTS --- */
    if (vfd_register("/dev/rw", rw_buf, sizeof(rw_buf), 1) != FD_EXISTS) {
        printk("FAIL: duplicate\n"); return;
    }
    printk("PASS: duplicate detection\n");

    /* --- path too long → FD_INVALID --- */
    if (vfd_register("/dev/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                     rw_buf, 1, 0) != FD_INVALID) {
        printk("FAIL: path too long\n"); return;
    }
    printk("PASS: path length guard\n");

    printk("=== vfd_test PASSED ===\n");
}
```

- [ ] **Step 2: Call `vfd_test` from `main`, before `init_start`**

In `src/main.c`, inside `main()`, add `vfd_test();` after `keyboard_start(100);` and before `init_start();`.

- [ ] **Step 3: Build and run**

```bash
make && make -C /home/gian/Documenti/Progetti/butos run
```

Expected: QEMU boots, screen shows all PASS lines and `=== vfd_test PASSED ===` before the shell (or before failure if `init_start` crashes because `/butosh` is still in root — that is fine at this stage).

- [ ] **Step 4: Commit**

```bash
git add src/main.c
git commit -m "test: in-kernel VFD smoke test (temporary, removed in next task)"
```

---

## Task 4: makefile — Unix directory tree

**Files:**
- Modify: `makefile` (top-level)

**Interfaces:**
- Produces: FAT32 partition with `/usr`, `/usr/bin`, `/bin`, `/etc`, `/tmp` created; all programs at `/usr/bin/*`

- [ ] **Step 1: Add `mmd` invocation after `mkfs.fat` line**

Current block in `makefile`:

```makefile
	rm -f $(PART)
	mkfs.fat -F 32 --mbr=y -C $(PART) 1000
	$(call color_text,91,"[MAKE] Compilazione degli eseguibili di base")
	$(MAKE) -C $(PROGDIR)
	$(call color_text,91,"[MAKE] Copia degli eseguibili nella partizione")
	mcopy -o -i $(PART) $(PROGBIN) ::/
```

Replace with:

```makefile
	rm -f $(PART)
	mkfs.fat -F 32 --mbr=y -C $(PART) 1000
	mmd -i $(PART) ::/usr ::/usr/bin ::/bin ::/etc ::/tmp
	$(call color_text,91,"[MAKE] Compilazione degli eseguibili di base")
	$(MAKE) -C $(PROGDIR)
	$(call color_text,91,"[MAKE] Copia degli eseguibili nella partizione")
	mcopy -o -i $(PART) $(PROGBIN) ::/usr/bin/
```

- [ ] **Step 2: Build and verify directory structure**

```bash
make
mdir -i /home/gian/Documenti/Progetti/butos/qemu/fat_part.img ::/usr/bin/
```

Expected: lists `butosh`, `ls`, `cat`, `touch`, `example`, `filetest`, `datatest`.

```bash
mdir -i /home/gian/Documenti/Progetti/butos/qemu/fat_part.img ::
```

Expected: lists `USR`, `BIN`, `ETC`, `TMP` directories (and no bare program names in root).

- [ ] **Step 3: Commit**

```bash
git add makefile
git commit -m "build: create Unix dir tree on FAT32, move programs to /usr/bin"
```

---

## Task 5: main.c — update INIT_PATH, remove test

**Files:**
- Modify: `src/main.c`

**Interfaces:**
- Consumes: `/usr/bin/butosh` now exists (Task 4)

- [ ] **Step 1: Change `INIT_PATH`**

```c
#define INIT_PATH   "/usr/bin/butosh"
```

- [ ] **Step 2: Remove the `vfd_test()` call and function body**

Delete the entire `static void vfd_test(void) { ... }` block and the `vfd_test();` call in `main()`.

- [ ] **Step 3: Build and boot**

```bash
make && make -C /home/gian/Documenti/Progetti/butos run
```

Expected: QEMU boots and presents the `butosh $` prompt (butosh found at `/usr/bin/butosh`).

- [ ] **Step 4: Commit**

```bash
git add src/main.c
git commit -m "kernel: update INIT_PATH to /usr/bin/butosh, remove VFD test"
```

---

## Task 6: butosh — PATH search

**Files:**
- Modify: `programs/sources/butosh/butosh.c`

**Interfaces:**
- Consumes: programs at `/usr/bin/` (Task 4), `exec_syscall` (existing)

- [ ] **Step 1: Widen `PATH_SIZE`**

Change:

```c
#define PATH_SIZE       (BUFFER_SIZE + 2)
```

To:

```c
#define PATH_SIZE       (BUFFER_SIZE + 16)
```

This ensures room for the `/usr/bin/` (9 char) prefix plus the full command name.

- [ ] **Step 2: Rewrite `program_execute` with PATH search**

```c
static int program_execute(struct command *cmd)
{
    static const char * const search_dirs[] = {
        "/usr/bin/", "/bin/", "/", NULL
    };
    char path[PATH_SIZE];
    char *argv[MAX_ARGS + 2];
    int exit_code = 0;
    int status = EXEC_NOT_FOUND;
    size_t len = strlen(cmd->name);
    int i;

    argv[0] = cmd->name;
    memcpy(&argv[1], cmd->args, cmd->argc * sizeof(char *));
    argv[cmd->argc + 1] = NULL;

    if (cmd->name[0] == '/') {
        /* Absolute path: use as-is */
        if (len + 1 > PATH_SIZE) {
            puts_syscall("Path too long\n");
            return EXIT_FAILURE;
        }
        memcpy(path, cmd->name, len + 1);
        status = exec_syscall(path, argv, &exit_code);
    } else {
        for (i = 0; search_dirs[i] != NULL && status == EXEC_NOT_FOUND; i++) {
            size_t prefix_len = strlen(search_dirs[i]);
            if (prefix_len + len + 1 > PATH_SIZE)
                continue;
            memcpy(path, search_dirs[i], prefix_len);
            memcpy(path + prefix_len, cmd->name, len + 1);
            status = exec_syscall(path, argv, &exit_code);
        }
    }

    switch (status) {
    case EXEC_OK:
        if (exit_code != EXIT_SUCCESS)
            printf_syscall("\n%s exited with code %d\n", cmd->name, exit_code);
        return exit_code;
    case EXEC_NOT_FOUND:
        printf_syscall("Unrecognized command: %s\n", cmd->name);
        break;
    case EXEC_INVALID:
        printf_syscall("%s: invalid executable\n", cmd->name);
        break;
    case EXEC_INVALID_ARGS:
        printf_syscall("%s: arguments too long\n", cmd->name);
        break;
    default:
        printf_syscall("%s: failed to execute\n", cmd->name);
        break;
    }
    return EXIT_FAILURE;
}
```

- [ ] **Step 3: Build and smoke-test**

```bash
make && make -C /home/gian/Documenti/Progetti/butos run
```

In QEMU:
```
butosh $ ls
butosh $ cat /etc
butosh $ touch /tmp/x
butosh $ /usr/bin/ls
```

Expected:
- `ls` works (found via `/usr/bin/ls`).
- `/usr/bin/ls` also works (absolute path).
- `cat /etc` fails gracefully (it's a directory, not a file — shows an error, does not crash).
- `unknowncmd` prints `Unrecognized command: unknowncmd`.

- [ ] **Step 4: Commit**

```bash
git add programs/sources/butosh/butosh.c
git commit -m "butosh: search /usr/bin, /bin, / for commands; widen PATH_SIZE"
```
