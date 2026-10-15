#ifndef PATH_H
#define PATH_H

#include <include/def.h>

/* Longest path (with the terminator) the kernel resolves */
#define PATH_MAX        256

/*
    Turns path, absolute or relative to cwd (absolute), into a normalized absolute path:
    "." and empty components are dropped, ".." removes the previous one (stopping at /),
    no trailing slash except for the root. Fails on an empty path or if out is too small.
*/
int path_resolve(const char *cwd, const char *path, char *out, size_t size);

#endif
