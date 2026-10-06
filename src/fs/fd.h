#ifndef FD_H
#define FD_H

#include <include/def.h>
#include <fs/fat.h>

/* Open files table shared by every process, used by the file syscalls */

#define FD_MAX          16

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

#endif
