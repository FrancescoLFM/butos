#include <cpu/paging.h>

static uint32_t page_directory[PAGE_DIR_SIZE] __attribute__((aligned(4096)));
static uint32_t kernel_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));

void page_directory_alloc() {
    for (size_t i = 0; i < PAGE_DIR_SIZE; i++) {
        page_directory[i] = KERNEL_PAGE_ATTR;
    }
}

/* Identity paging */
void kernel_page_table_alloc() {
    for (size_t i = 0; i < PAGE_TABLE_SIZE; i++) {
        kernel_page_table[i] = (PAGE_FRAME_SIZE * i) | KERNEL_PAGE_ATTR;
    }

    page_directory[KERNEL_PAGE] = (uint32_t) (kernel_page_table) | KERNEL_PAGE_ATTR;
}

void paging_init() {
    page_directory_alloc();
    kernel_page_table_alloc();
    page_directory_load(page_directory);
    paging_enable();
}
