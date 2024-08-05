#include <cpu/paging.h>

struct page_table {
    uint32_t *table;
    size_t size;
    uint8_t attr;  
};

static uint32_t page_directory[PAGE_DIR_SIZE] __attribute__((aligned(4096)));
static uint32_t _identity_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));
static uint32_t _kernel_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));

static struct page_table kernel_page_table = {
    .table = _kernel_page_table
};
static struct page_table identity_page_table = {
    .table = _identity_page_table
};

void page_directory_alloc() {
    for (size_t i = 0; i < PAGE_DIR_SIZE; i++) {
        page_directory[i] = KERNEL_PAGE_ATTR;
    }
}

void page_table_alloc(struct page_table *page_table, uint32_t physical_start, uint32_t virtual_start, uint32_t page_size, uint8_t attr)
{
    page_table->size = page_size;
    page_table->attr = attr;
    for (uint32_t i = 0; i < page_size; i++) {
        page_table->table[i] = ((PAGE_FRAME_SIZE * i) + physical_start) | attr;
    }
    //? virtual_start = index * 0x400000
    page_directory[virtual_start / INDEX_TO_ADDR] = (uintptr_t) (page_table->table) | attr;
}

void paging_init() {
    page_directory_alloc();
    /* Identity paging the kernel bootloader */ 
    page_table_alloc(&identity_page_table, IDENTITY_PAGE_START, IDENTITY_PAGE_START, IDENTITY_PAGE_SIZE, KERNEL_PAGE_ATTR);
    /* Paging the kernel */
    page_table_alloc(&kernel_page_table, KERNEL_PHYSICAL_START, KERNEL_VIRTUAL_START, KERNEL_PAGE_SIZE, KERNEL_PAGE_ATTR);
    page_directory_load(page_directory);
    paging_enable();
}
