#include <cpu/vmm.h>
#include <cpu/paging.h>
#include <libs/alloc.h>
#include <libs/string.h>

/* Page tables allocated by this module live in the kernel heap */
static uint32_t *page_table_from_entry(uint32_t entry)
{
    return (uint32_t *) ((entry & PAGE_ADDR_MASK) - KERNEL_PHYSICAL_START + KERNEL_VIRTUAL_START);
}

int paging_map_range(uint32_t *page_dir, const uint32_t *kernel_dir, uintptr_t vaddr,
                     uintptr_t paddr, size_t size, uint8_t attr)
{
    uint32_t *table;
    size_t dir_index, table_index;

    for (size_t off = 0; off < size; off += PAGE_FRAME_SIZE) {
        dir_index = (vaddr + off) / INDEX_TO_ADDR;
        table_index = ((vaddr + off) / PAGE_FRAME_SIZE) % PAGE_TABLE_SIZE;

        if (kernel_dir[dir_index] & PAGE_PRESENT)
            return EXIT_FAILURE;

        if (page_dir[dir_index] & PAGE_PRESENT) {
            table = page_table_from_entry(page_dir[dir_index]);
        } else {
            table = aligned_kalloc(PAGE_FRAME_SIZE, PAGE_TABLE_SIZE * sizeof(*table));
            if (table == NULL)
                return EXIT_FAILURE;
            memset(table, 0, PAGE_TABLE_SIZE * sizeof(*table));
            page_dir[dir_index] = kernel_virtual_to_physical((uintptr_t) table) | attr;
        }
        table[table_index] = (paddr + off) | attr;
    }

    return EXIT_SUCCESS;
}

void paging_free_tables(uint32_t *page_dir, const uint32_t *kernel_dir)
{
    for (size_t i = 0; i < PAGE_DIR_SIZE; i++) {
        if ((page_dir[i] & PAGE_PRESENT) && page_dir[i] != kernel_dir[i]) {
            kfree(page_table_from_entry(page_dir[i]));
            page_dir[i] = kernel_dir[i];
        }
    }
}
