#ifndef ALLOC_H
#define ALLOC_H

#include <include/def.h>

void kalloc_start(
    void *heap,
    size_t heap_size,
    void *registry,
    size_t reg_size
);
void *kalloc(size_t size);
void kfree(void *ptr);
void *kcalloc(size_t nmemb, size_t size);
void *krealloc(void *ptr, size_t array);
void kalloc_print();

#endif