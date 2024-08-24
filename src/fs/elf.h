#ifndef ELF_H
#define ELF_H

#include <fs/fat.h>
#include <include/def.h>

#define ELF32_HEADER_SIZE 52
#define ELF_MAGIC_NUMBER  0x7F
#define ELF_STRING        "ELF"
#define ELF_STRING_SIZE   sizeof(ELF_STRING) - 1

typedef enum {
    ELF_SUCCESS,
    ELF_GENERIC_ERROR,
    NOT_ELF,
    ELF_UNSUPPORTED_ARCH,
    ELF_MALLOC_ERROR
} elf_status_t;

enum {
    ELF_GENERIC_ARCH=0,
    ELF_SPARC=0x02,
    ELF_X86=0x03,
    ELF_MIPS=0x08,
    ELF_POWERPC=0x14,
    ELF_ARM=0x28,
    ELF_SUPERH=0x2A,
    ELF_IA_64=0x32,
    ELF_X86_64=0x3E,
    AArch64=0xB7,
    RISV_V=0xF3,
};

enum {
    ELF32_ARCH=1,
    ELF64_ARCH=2,
    ELF_LITTLE_ENDIAN=1,
    ELF_BIG_ENDIAN=2
};

typedef enum {
    ELF_MAGIC_NUMBER_OFF=0,
    ELF_ARCH_OFF=4,
    ELF_ENDIANESS_OFF=5,
    ELF_HEADER_VERSION_OFF=6,
    ELF_TYPE_OFF = 16,
    ELF_INSSET_OFF = 18,
    ELF_VERSION_OFF = 20,
    ELF_P_ENTRY_OFF=24,
    ELF_P_HEADER_OFF=28,
    ELF_S_HEADER_OFF=32,
} elf_header_offsets_t;

struct elf_header {
    u32 magic_numbers;
    u8 arch;
    u8 endianess;
    u8 header_version;
    u8 os_abi;
    u8 padding[8];
    u16 type;
    u16 insset;
    u32 elf_version;
    u32 p_entry_offset;
    u32 p_header_offset;
    u32 s_header_offset;
    u32 flags;
    u16 size;
    u16 p_entry_size;
    u16 p_entry_num;
    u16 s_entry_size;
    u16 s_entry_num;
    u16 str_index;
};

typedef enum {
    PT_NULL,
    PT_LOAD,
    PT_DYNAMIC,
    PT_INTERP,
    PT_NOTE_SEC,
} pt_t;
struct elf_p_header {
    pt_t segment_type;
    u32 p_offset;
    u32 p_vaddr;
    u32 p_paddr;
    u32 p_filez;
    u32 p_memsz;
    u32 flags;
    u32 alignment;
};

struct elf {
    file_t *file;
    struct elf_header header;
    struct elf_p_header *p_headers;
};

typedef struct elf elf_t;

elf_status_t elf_init(elf_t *elf, file_t *file, fat_fs_t *fs);
void elf_fini(elf_t *elf);
elf_status_t p_header_memload(struct elf_p_header *p_header, elf_t *elf);

#endif