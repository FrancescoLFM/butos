#ifndef PROCFS_H
#define PROCFS_H

#include <fs/fd.h>

/*
    Virtual files under /proc generated from the process table, hooked into the fd
    layer (the FAT32 partition only has an empty /proc directory to show it in /):

    /proc/self              the calling process (link to its /proc/<pid>)
    /proc/<pid>/status      process table entry as text
    /proc/<pid>/cmdline     arguments separated by spaces
    /proc/<pid>/fd/<fd>     opening it opens what <fd> refers to (0, 1, 2 are the
                            console streams, the others the files <pid> opened)

    Files are read only snapshots taken on open.
*/

#define PROCFS_ROOT         "/proc"

/* Whether path is /proc or below it */
int procfs_match(const char *path);
/* Same contract as fd_open */
int procfs_open(const char *path);
/* Same contract as fd_readdir */
int procfs_readdir(const char *path, int index, fd_dirent_t *out);

#endif
