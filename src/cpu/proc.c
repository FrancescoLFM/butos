#include <cpu/proc.h>
#include <include/asm.h>
#include <libs/allocator.h>
#include <libs/alloc.h>
#include <libs/print.h>
#include <libs/scan.h>
#include <cpu/paging.h>
#include <cpu/vmm.h>
#include <libs/string.h>

/* Physical memory reserved to process images */
#define PROC_PMEM_START     0x01000000
#define PROC_PMEM_SIZE      0x0F000000
#define INIT_REGISTRY_SIZE  (200 * sizeof(struct memspace))
/* Processes can't be mapped over the identity mapped area or the kernel */
#define PROC_VMEM_START     INDEX_TO_ADDR
#define PROC_VMEM_END       KERNEL_VIRTUAL_START

static struct allocator proc_allocator;
static int proc_initialized = 0;

int process_init() 
{
    void *registry;

    if (proc_initialized)
        return EXIT_SUCCESS;

    registry = kalloc(INIT_REGISTRY_SIZE);
    if (registry == NULL)
        return EXIT_FAILURE;
    if (allocator_init(&proc_allocator, PROC_PMEM_START, PROC_PMEM_SIZE, registry, INIT_REGISTRY_SIZE, PAGE_FRAME_SIZE)) {
        kfree(registry);
        return EXIT_FAILURE;
    }
    proc_initialized = 1;

    return EXIT_SUCCESS;
}

/* Page aligned virtual span covering every loadable segment */
static int process_image_span(elf_t *elf, uintptr_t *start, uintptr_t *end)
{
    struct elf_p_header *p_header;
    uintptr_t seg_end;

    *start = UINTPTR_MAX;
    *end = 0;
    for (u16 i = 0; i < elf->header.p_entry_num; i++) {
        p_header = &elf->p_headers[i];
        if (p_header->segment_type != PT_LOAD || p_header->p_memsz == 0)
            continue;
        seg_end = p_header->p_vaddr + p_header->p_memsz;
        if (seg_end < p_header->p_vaddr || p_header->p_filez > p_header->p_memsz)
            return EXIT_FAILURE;
        if (p_header->p_vaddr < *start)
            *start = p_header->p_vaddr;
        if (seg_end > *end)
            *end = seg_end;
    }
    if (*start >= *end)
        return EXIT_FAILURE;

    *start = PAGE_ROUND_DOWN(*start);
    *end = PAGE_ROUND_UP(*end);
    if (*start < PROC_VMEM_START || *end > PROC_VMEM_END || *end == 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

/* The program runs in ring 0 on the kernel stack and follows the cdecl ABI */
static int process_call(uintptr_t entry)
{
    int ret;

    __asm__ __volatile__ (
        "call *%1"
        : "=a"(ret)
        : "r"(entry)
        : "ecx", "edx", "memory", "cc"
    );

    return ret;
}

int process_exec(elf_t *elf, int *exit_code)
{
    uint32_t *kernel_dir;
    uint32_t *page_dir;
    uintptr_t vstart, vend, paddr;
    uintptr_t entry = elf->header.p_entry_offset;
    int status = EXIT_FAILURE;

    if (!proc_initialized || process_image_span(elf, &vstart, &vend))
        return EXIT_FAILURE;
    if (entry < vstart || entry >= vend)
        return EXIT_FAILURE;

    /* The whole image is physically contiguous, so segments sharing a page are fine */
    paddr = allocator_alloc(&proc_allocator, vend - vstart);
    if (paddr == 0)
        return EXIT_FAILURE;

    page_dir = aligned_kalloc(PAGE_FRAME_SIZE, PAGE_DIR_SIZE * sizeof(*page_dir));
    if (page_dir == NULL)
        goto free_image;
    kernel_dir = get_blank_page_directory();
    memcpy(page_dir, kernel_dir, PAGE_DIR_SIZE * sizeof(*page_dir));
    if (paging_map_range(page_dir, kernel_dir, vstart, paddr, vend - vstart, KERNEL_PAGE_ATTR))
        goto free_dir;

    page_directory_load((uint32_t *) kernel_virtual_to_physical((uintptr_t) page_dir));
    for (u16 i = 0; i < elf->header.p_entry_num; i++) {
        if (elf->p_headers[i].segment_type == PT_LOAD && elf->p_headers[i].p_memsz)
            if (p_header_memload(&elf->p_headers[i], elf))
                goto restore_dir;
    }
    *exit_code = process_call(entry);
    status = EXIT_SUCCESS;

restore_dir:
    /* The kernel page directory is identity mapped, its address is also physical */
    page_directory_load(kernel_dir);
free_dir:
    paging_free_tables(page_dir, kernel_dir);
    kfree(page_dir);
free_image:
    allocator_free(&proc_allocator, paddr);

    return status;
}
