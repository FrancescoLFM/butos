#include "allocator.h"

#include <stdint.h>
#include <libs/string.h>

allocator_t kallocator_;
static allocator_t *kallocator = &kallocator_;

void kalloc_start(
    void *heap,
    size_t heap_size,
    void *registry,
    size_t reg_size
)
{
    allocator_init(
        kallocator,
        heap,
        heap_size,
        registry,
        reg_size
    );
}

void *kalloc(size_t size)
{
    return allocator_alloc(kallocator, size);
}

void kfree(void *ptr)
{
    allocator_free(kallocator, ptr);
}

void *kcalloc(size_t nmemb, size_t size)
{
    void *ptr;
    ptr = allocator_alloc(kallocator, size * nmemb);
    if (!ptr)
        return NULL;
    memset(ptr, 0, size * nmemb);
    return ptr;
}

void *krealloc(void *ptr, size_t size)
{
    return allocator_realloc(kallocator, ptr, size);
}

void kalloc_print()
{
    allocator_print(kallocator, 0);
}