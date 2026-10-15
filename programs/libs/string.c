/*
 * The kernel string library compiled for user programs. Its includes
 * resolve against programs/: <libs/string.h> is the wrapper above and
 * <libs/alloc.h> maps kalloc/kfree on the heap syscalls.
 */
#include "../../src/libs/string.c"
