#include <cpu/proc.h>
#include <include/asm.h>
#include <libs/allocator.h>
#include <libs/alloc.h>
#include <libs/print.h>
#include <libs/scan.h>
#include <cpu/paging.h>
#include <libs/string.h>

static struct allocator proc_allocator;
#define INIT_VMEM_START     0x01000000
#define INIT_VMEM_SIZE      (KERNEL_VIRTUAL_START - INIT_VMEM_START)
#define INIT_REGISTRY_SIZE  200 * sizeof(uint32_t)

int process_init() 
{
    int errno;
    void *registry;

    registry = kalloc(INIT_REGISTRY_SIZE);
    errno = allocator_init(&proc_allocator, INIT_VMEM_START, INIT_VMEM_SIZE, (void *) registry, INIT_REGISTRY_SIZE, 1);
    
    return errno;
}

int process_exec(elf_t *elf)
{
    uintptr_t paddr;
    uint32_t *page_directory_blank;

    uint32_t page_directory_new[PAGE_DIR_SIZE] __attribute__((aligned(4096)));
    uint32_t _proc_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));

    page_directory_blank = get_blank_page_directory();
    memcpy(page_directory_new, page_directory_blank, PAGE_DIR_SIZE * sizeof(uint32_t));
    struct page_table proc_page_table = {
        .table = _proc_page_table
    };
    for (u16 i = 0; i < elf->header.p_entry_num; i++) {
        if (elf->p_headers[i].segment_type == PT_LOAD && elf->p_headers[i].p_offset) {
            paddr = allocator_alloc(&proc_allocator, elf->p_headers[i].p_memsz);
            kernel_page_table_alloc(page_directory_new, &proc_page_table, paddr, elf->p_headers[i].p_vaddr, PAGE_TABLE_SIZE, KERNEL_PAGE_ATTR);
            page_directory_load((uint32_t *) kernel_virtual_to_physical((uintptr_t) page_directory_new));
            p_header_memload(&elf->p_headers[i], elf);
        }
    }
    pusha();
    call(elf->header.p_entry_offset);
    popa();
    // stack context switching
    // jump to the entry, return

    // kfree(page_dummy);

    return 0;
}
