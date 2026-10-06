#ifndef VMM_H
#define VMM_H

#include <include/def.h>
#include <cpu/paging.h>

#define PAGE_PRESENT            0x1
#define PAGE_ADDR_MASK          0xFFFFF000
#define PAGE_ROUND_DOWN(addr)   ((addr) & PAGE_ADDR_MASK)
#define PAGE_ROUND_UP(addr)     PAGE_ROUND_DOWN((addr) + PAGE_FRAME_SIZE - 1)

/*
 * Kernel-only paging helpers (they use the kernel heap, so they can't live in
 * paging.c, which is also linked into the second stage bootloader).
 */

/* Maps [vaddr, vaddr + size) to [paddr, paddr + size) in page_dir, allocating
 * page tables on demand. Fails if a page directory entry is already used by
 * kernel_dir, since those page tables are shared with the kernel. */
int paging_map_range(uint32_t *page_dir, const uint32_t *kernel_dir, uintptr_t vaddr,
                     uintptr_t paddr, size_t size, uint8_t attr);
/* Frees the page tables of page_dir that are not shared with kernel_dir */
void paging_free_tables(uint32_t *page_dir, const uint32_t *kernel_dir);

#endif
