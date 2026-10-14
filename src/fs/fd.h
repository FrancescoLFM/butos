#ifndef FD_H
#define FD_H

#include <include/def.h>
#include <fs/fat.h>

/* Open files table shared by every process, used by the file syscalls */

#define FD_MAX          16

/* Standard streams, always open on the console: they can't be closed */
#define FD_STDIN        0
#define FD_STDOUT       1
#define FD_STDERR       2
#define FD_STD_NUM      3

#define VFD_MAX         8
#define VFD_PATH_MAX    64

typedef enum {
    FD_TYPE_FAT32,
    FD_TYPE_RAM,
    FD_TYPE_CONSOLE,
} fd_type_t;

#define FD_SEEK_SET     0
#define FD_SEEK_CUR     1
#define FD_SEEK_END     2

/* Negative so that read/write/seek can return either a count or an error */
typedef enum {
    FD_OK = 0,
    FD_NOT_FOUND = -1,
    FD_INVALID = -2,
    FD_EXISTS = -3,
    FD_TOO_MANY = -4,
    FD_BUSY = -5,
    FD_IO_ERROR = -6,
} fd_status_t;

void fd_init(fat_fs_t *root_fs);
/* Returns the new descriptor or an fd_status_t error */
int fd_open(char *path);
fd_status_t fd_close(int fd);
/* Return the bytes transferred or an fd_status_t error */
int fd_read(int fd, void *buffer, size_t size);
int fd_write(int fd, void *buffer, size_t size);
/* Returns the new position or an fd_status_t error, can't move past the end of the file */
int fd_seek(int fd, int offset, int whence);
fd_status_t fd_create(char *path);
fd_status_t fd_remove(char *path);
/* Closes the descriptors opened at process nesting depth owner or deeper */
void fd_close_owned(int owner);

/* Entry returned by fd_readdir; name is decoded 8.3, lowercased, NUL-terminated */
typedef struct {
    char     name[13];
    int      is_dir;
    uint32_t size;
} fd_dirent_t;

/* Returns 1 if entry at index exists (fills *out), 0 at end-of-dir, negative fd_status_t on error.
 * Skips deleted, LFN, and dot entries. index is 0-based over visible entries. */
int fd_readdir(char *path, int index, fd_dirent_t *out);

/* Opens a read only descriptor on buffer, a kalloc block freed when it's closed (also on failure) */
int fd_open_buffer(const char *path, void *buffer, size_t size);
/* Opens a new descriptor on what fd refers to, from the start (e.g. through /proc/<pid>/fd/<fd>) */
int fd_reopen(int fd);
/* Process nesting depth that opened fd (0 for the standard streams) or an fd_status_t error */
int fd_owner(int fd);

/* Register/unregister RAM-backed virtual files.
 * vfd_register may be called before or after fd_init.
 * vfd_unregister fails with FD_BUSY if the file is open. */
fd_status_t vfd_register(const char *path, void *buf, size_t size, int writable);
fd_status_t vfd_unregister(const char *path);

#endif
