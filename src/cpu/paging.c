#include <cpu/paging.h>

static uint32_t _page_directory[PAGE_DIR_SIZE] __attribute__((aligned(4096)));
static uint32_t *page_directory;
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
        _page_directory[i] = KERNEL_PAGE_ATTR;
    }
    page_directory = _page_directory;
}

void page_table_init(struct page_table *page_table, uint32_t physical_start, uint32_t page_size, uint8_t attr)
{
    page_table->size = page_size;
    page_table->attr = attr;
    for (uint32_t i = 0; i < page_size; i++) {
        page_table->table[i] = ((PAGE_FRAME_SIZE * i) + physical_start) | attr;
    }
}

void page_table_alloc(struct page_table *page_table, uint32_t physical_start, uint32_t virtual_start, uint32_t page_size, uint8_t attr)
{
    page_table_init(page_table, physical_start, page_size, attr);
    //? virtual_start = index * 0x400000
    page_directory[virtual_start / INDEX_TO_ADDR] = (uintptr_t) (page_table->table) | attr;
}

void __attribute__((noinline)) page_directory_adjust()
{
    __asm__ __volatile__ (
        "mov %%cr3, %0"
        : "=r"(page_directory)
        :
    );
}

uint32_t *get_blank_page_directory()
{
    return page_directory;
}

uintptr_t kernel_virtual_to_physical(uintptr_t virtual_address) {
    // Restituisci l'indirizzo fisico completo aggiungendo l'offset
    return virtual_address - KERNEL_VIRTUAL_START + KERNEL_PHYSICAL_START;
}

void kernel_page_table_alloc(uint32_t *page_dir, struct page_table *page_table, uint32_t physical_start, uint32_t virtual_start, uint32_t page_size, uint8_t attr)
{
    page_table_init(page_table, physical_start, page_size, attr);
    //? virtual_start = index * 0x400000
    page_dir[virtual_start / INDEX_TO_ADDR] = kernel_virtual_to_physical((uintptr_t) page_table->table) | attr;
}


void paging_init() {
    page_directory_alloc();
    page_directory_load(page_directory);
    /* Identity paging the kernel bootloader */ 
    page_table_alloc(&identity_page_table, IDENTITY_PAGE_START, IDENTITY_PAGE_START, IDENTITY_PAGE_SIZE, KERNEL_PAGE_ATTR);
    /* Paging the kernel */
    page_table_alloc(&kernel_page_table, KERNEL_PHYSICAL_START, KERNEL_VIRTUAL_START, KERNEL_PAGE_SIZE, KERNEL_PAGE_ATTR);
    paging_enable();
}
