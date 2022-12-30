#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <include/def.h>

struct memspace {
    void *start;
    size_t size;
};

struct allocator {
    struct memspace heap;      /* - place where blocks are allocated */
    struct memspace *registry; /* - keep tracks of allocated space */
    size_t size;               /* - number of blocks allocated at each moment */
    size_t capacity;           /* - maximum number of blocks we can allocate */
};

typedef struct allocator allocator_t;

void allocator_init(
    allocator_t *a,
    void *heap,
    size_t heap_size,
    void *registry,
    size_t reg_size
);

int allocator_full(allocator_t *a);
void *allocator_alloc(allocator_t *a, size_t size);
void *allocator_realloc(allocator_t *a, void *ptr, size_t size);
void allocator_free(allocator_t *a, void *ptr);
void allocator_print(allocator_t *a, int verbose); /* debug */

#endif