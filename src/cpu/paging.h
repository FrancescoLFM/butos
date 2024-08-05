#ifndef PAGING_H
#define PAGING_H

#include <include/def.h>

#define IDENTITY_PAGE_START     0x0
#define PAGE_DIR_SIZE           1024
#define PAGE_TABLE_SIZE         1024
#define IDENTITY_PAGE_SIZE      200
#define KERNEL_PAGE_SIZE        1024
#define KERNEL_PAGE_ATTR        0b11
#define PAGE_FRAME_SIZE         0x1000
#define HIGH_KERNEL_ADDR        0x300
#define KERNEL_PHYSICAL_START   0x100000
#define KERNEL_VIRTUAL_START    0xC0000000
#define LOW_KERNEL_PAGE         0
#define HIGH_KERNEL_PAGE        1

#define INDEX_TO_ADDR           0x400000

void page_directory_alloc();
void kernel_page_table_alloc();

extern void page_directory_load(uint32_t *page_directory_addr);
extern void paging_enable();

void paging_init();

#endif