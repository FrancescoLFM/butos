#include <fs/fat.h>
#include <fs/elf.h>
#include <libs/alloc.h>
#include <libs/string.h>
#include <libs/print.h>

static fat_fs_t *current_filesystem;

void print_elf_header_info(struct elf_header *header)
{
    puts("Architecture: ");
    if (header->arch == ELF32_ARCH)
        puts("ELF32\n"); 
    else if (header->arch == ELF64_ARCH)
        puts("ELF64\n");
    puts("Endianess: ");
    if (header->type == ELF_LITTLE_ENDIAN)
        puts("Little endian\n"); 
    else if (header->type == ELF_BIG_ENDIAN)
        puts("Big endian\n");
    printk("Version: %d\n", header->elf_version);
    puts("Instruction set: ");
    if (header->insset == ELF_X86)
        puts("Intel X86\n");
    else
        puts("Generic\n");
    printk("Program entry point address: 0x%x\n", header->p_entry_offset); 
    printk("Program header offset: %d\n", header->p_header_offset);
    printk("Section header offset: %d\n", header->s_header_offset);
    printk("Size of this header: %d (bytes)\n", header->size);
    printk("Size of program headers: %d (bytes)\n", header->p_entry_size);
    printk("Number of program headers: %d\n", header->p_entry_num);
    printk("Size of section headers: %d (bytes)\n", header->s_entry_size);
    printk("Number of section headers: %d\n", header->s_entry_num);
    printk("Section header string table index: %d\n", header->str_index);
}

elf_status_t elf_read_header(elf_t *elf)
{
    uint8_t *elf_header_raw;

    elf_header_raw = file_read(elf->file, current_filesystem, 0, ELF32_HEADER_SIZE);
    if (elf_header_raw[ELF_MAGIC_NUMBER_OFF] != ELF_MAGIC_NUMBER && 
        strncmp((char *)elf_header_raw + 1, ELF_STRING, ELF_STRING_SIZE))
        return NOT_ELF;

    memcpy(&elf->header, elf_header_raw, ELF32_HEADER_SIZE);

    kfree(elf_header_raw);

    return ELF_SUCCESS;
}

void print_buffer(uint8_t *buffer, size_t size)
{
    uint32_t words_readed = 0;
    for (uint32_t i=0; i < size; i++) {
        printk("%x ", buffer[i]);
        
        if (++words_readed == 30) {
            words_readed = 0;
            putc('\n');
        }
    }

    putc('\n');
}

elf_status_t p_header_memload(struct elf_p_header *p_header, elf_t *elf)
{
    void *v_addr;
    uint8_t *pmem_raw;

    v_addr = (void *) p_header->p_vaddr;
    pmem_raw = file_read(elf->file, current_filesystem, p_header->p_offset, p_header->p_memsz);
    memcpy(v_addr, pmem_raw, p_header->p_memsz);

    kfree(pmem_raw);

    return ELF_SUCCESS;
}

void print_p_header(struct elf_p_header *p_header)
{
    char *type_str[] = {"NULL", "LOAD", "DYNAMIC", "INTERP", "NOTE"};
    if (p_header->segment_type > 4)
        return;
    printk("%s\t0x%x\t0x%x\t0x%x\n", type_str[p_header->segment_type], p_header->p_offset, p_header->p_vaddr, p_header->p_memsz);
}

elf_status_t elf_read_p_headers(elf_t *elf)
{
    uint8_t *p_header_raw;
    
    elf->p_headers = kcalloc(elf->header.p_entry_num, sizeof(*elf->p_headers));
    if (elf->p_headers == NULL)
        return ELF_MALLOC_ERROR;

    for (u16 i = 0; i < elf->header.p_entry_num; i++) {
        p_header_raw = file_read(elf->file, current_filesystem, 
                                 elf->header.p_header_offset + (elf->header.p_entry_size * i), 
                                 elf->header.p_entry_size);
        memcpy(&elf->p_headers[i], p_header_raw, elf->header.p_entry_size);

        kfree(p_header_raw);
    }

    return ELF_SUCCESS;
}

/* Assumes that file and fs are not NULL */
elf_status_t elf_init(elf_t *elf, file_t *file, fat_fs_t *fs)
{
    elf_status_t errno;

    current_filesystem = fs;
    elf->file = file;
    if((errno = elf_read_header(elf)))
        return errno;
    if (elf->header.arch != ELF32_ARCH && elf->header.insset != ELF_X86)
        return ELF_UNSUPPORTED_ARCH;
    if((errno = elf_read_p_headers(elf)))
        return errno;

    return errno;
}

void elf_fini(elf_t *elf)
{
    kfree(elf->p_headers);
}