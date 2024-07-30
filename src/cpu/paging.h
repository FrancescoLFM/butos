#ifndef PAGING_H
#define PAGING_H

#include <include/def.h>

#define PAGE_DIR_SIZE       1024
#define PAGE_TABLE_SIZE     1024
#define KERNEL_PAGE_ATTR    0b11
#define PAGE_FRAME_SIZE     0x1000
#define KERNEL_PAGE         0

void page_directory_alloc();
void kernel_page_table_alloc();

extern void page_directory_load(uint32_t *page_directory_addr);
extern void paging_enable();

void paging_init();

#endif