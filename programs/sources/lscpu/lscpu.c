#include <include/syscalls.h>
#include <libs/string.h>

#define LABEL_WIDTH     22
#define SCREEN_WIDTH    80

#define LEAF_VENDOR         0x00000000
#define LEAF_FEATURES       0x00000001
#define LEAF_CACHE          0x00000004
#define LEAF_EXT_FEATURES   0x00000007
#define LEAF_HYPERVISOR     0x40000000
#define LEAF_EXT_MAX        0x80000000
#define LEAF_EXT_FLAGS      0x80000001
#define LEAF_BRAND          0x80000002
#define LEAF_AMD_L1         0x80000005
#define LEAF_AMD_L2_L3      0x80000006
#define LEAF_ADDR_SIZES     0x80000008

#define FEAT_EDX_HTT        BIT(28)
#define FEAT_ECX_HYPERVISOR BIT(31)
#define EXT_EDX_LM          BIT(29)

enum { EAX, EBX, ECX, EDX };

/* Flag names as in Linux /proc/cpuinfo, indexed by bit (NULL for reserved or not shown) */
static const char *flags_1_edx[32] = {
    "fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce", "cx8", "apic", NULL, "sep",
    "mtrr", "pge", "mca", "cmov", "pat", "pse36", "pn", "clflush", NULL, "dts", "acpi", "mmx",
    "fxsr", "sse", "sse2", "ss", "ht", "tm", "ia64", "pbe",
};

static const char *flags_1_ecx[32] = {
    "pni", "pclmulqdq", "dtes64", "monitor", "ds_cpl", "vmx", "smx", "est", "tm2", "ssse3",
    "cnxt-id", "sdbg", "fma", "cx16", "xtpr", "pdcm", NULL, "pcid", "dca", "sse4_1", "sse4_2",
    "x2apic", "movbe", "popcnt", "tsc_deadline_timer", "aes", "xsave", NULL, "avx", "f16c",
    "rdrand", "hypervisor",
};

static const char *flags_7_ebx[32] = {
    [0] = "fsgsbase", [3] = "bmi1", [4] = "hle", [5] = "avx2", [7] = "smep", [8] = "bmi2",
    [9] = "erms", [10] = "invpcid", [11] = "rtm", [16] = "avx512f", [17] = "avx512dq",
    [18] = "rdseed", [19] = "adx", [20] = "smap", [23] = "clflushopt", [24] = "clwb",
    [29] = "sha_ni",
};

static const char *flags_ext_edx[32] = {
    [11] = "syscall", [20] = "nx", [22] = "mmxext", [25] = "fxsr_opt", [26] = "pdpe1gb",
    [27] = "rdtscp", [29] = "lm", [30] = "3dnowext", [31] = "3dnow",
};

static const char *flags_ext_ecx[32] = {
    [0] = "lahf_lm", [1] = "cmp_legacy", [2] = "svm", [3] = "extapic", [4] = "cr8_legacy",
    [5] = "abm", [6] = "sse4a", [7] = "misalignsse", [8] = "3dnowprefetch", [9] = "osvw",
    [10] = "ibs", [11] = "xop", [12] = "skinit", [13] = "wdt", [15] = "lwp", [16] = "fma4",
    [17] = "tce", [21] = "tbm", [22] = "topoext",
};

static void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t regs[4])
{
    __asm__ __volatile__ (
        "cpuid"
        : "=a"(regs[EAX]), "=b"(regs[EBX]), "=c"(regs[ECX]), "=d"(regs[EDX])
        : "a"(leaf), "c"(subleaf)
    );
}

static char *utoa(unsigned int num, char *buf, size_t size)
{
    char *p = buf + size - 1;

    *p = '\0';
    do {
        *--p = (char) ('0' + num % 10);
        num /= 10;
    } while (num && p > buf);

    return p;
}

static void label(const char *name)
{
    int len = (int) strlen(name);

    puts_syscall((char *) name);
    putc_syscall(':');
    for (len++; len < LABEL_WIDTH; len++)
        putc_syscall(' ');
}

static void field(const char *name, const char *value)
{
    label(name);
    puts_syscall((char *) value);
    putc_syscall('\n');
}

static void field_num(const char *name, unsigned int num, const char *suffix)
{
    char buf[12];

    label(name);
    puts_syscall(utoa(num, buf, sizeof(buf)));
    puts_syscall((char *) suffix);
    putc_syscall('\n');
}

/* 12 characters vendor strings are spread over three registers */
static void regs_string(char *out, uint32_t a, uint32_t b, uint32_t c)
{
    memcpy(out, &a, 4);
    memcpy(out + 4, &b, 4);
    memcpy(out + 8, &c, 4);
    out[12] = '\0';
}

static void print_brand(uint32_t ext_max)
{
    uint32_t regs[4];
    char brand[49];
    char *start = brand;

    if (ext_max < LEAF_BRAND + 2)
        return;
    for (uint32_t i = 0; i < 3; i++) {
        cpuid(LEAF_BRAND + i, 0, regs);
        memcpy(brand + i * 16, regs, 16);
    }
    brand[48] = '\0';
    while (*start == ' ')
        start++;
    field("Model name", start);
}

/* Intel deterministic cache parameters */
static void print_caches_intel(void)
{
    static const char *types[] = {NULL, "d", "i", ""};
    uint32_t regs[4];
    uint32_t type, level, size;
    char name[16];

    for (uint32_t i = 0; ; i++) {
        cpuid(LEAF_CACHE, i, regs);
        type = regs[EAX] & 0x1F;
        if (type == 0 || type >= ARR_SIZE(types))
            break;
        level = (regs[EAX] >> 5) & 0x7;
        /* ways * partitions * line size * sets */
        size = ((regs[EBX] >> 22) + 1) * (((regs[EBX] >> 12) & 0x3FF) + 1) *
               ((regs[EBX] & 0xFFF) + 1) * (regs[ECX] + 1);
        strcpy(name, "L");
        name[1] = (char) ('0' + level);
        name[2] = '\0';
        strcat(name, types[type]);
        strcat(name, " cache");
        field_num(name, size / 1024, " KiB");
    }
}

/* AMD (and QEMU's default CPU) extended cache leaves */
static void print_caches_amd(uint32_t ext_max)
{
    uint32_t regs[4];

    if (ext_max >= LEAF_AMD_L1) {
        cpuid(LEAF_AMD_L1, 0, regs);
        if (regs[ECX] >> 24)
            field_num("L1d cache", regs[ECX] >> 24, " KiB");
        if (regs[EDX] >> 24)
            field_num("L1i cache", regs[EDX] >> 24, " KiB");
    }
    if (ext_max >= LEAF_AMD_L2_L3) {
        cpuid(LEAF_AMD_L2_L3, 0, regs);
        if (regs[ECX] >> 16)
            field_num("L2 cache", regs[ECX] >> 16, " KiB");
        if (regs[EDX] >> 18)
            field_num("L3 cache", (regs[EDX] >> 18) * 512, " KiB");
    }
}

/* Prints the flags set in reg, wrapping at the screen width, column tracks the cursor */
static void print_flags(const char *names[32], uint32_t reg, int *column)
{
    int len;

    for (int bit = 0; bit < 32; bit++) {
        if (!(reg & BIT(bit)) || names[bit] == NULL)
            continue;
        len = (int) strlen(names[bit]);
        if (*column + 1 + len >= SCREEN_WIDTH) {
            putc_syscall('\n');
            for (*column = 0; *column < LABEL_WIDTH; (*column)++)
                putc_syscall(' ');
        } else if (*column > LABEL_WIDTH) {
            putc_syscall(' ');
            (*column)++;
        }
        puts_syscall((char *) names[bit]);
        *column += len;
    }
}

int main(void)
{
    uint32_t regs[4], features[4], ext[4] = {0}, ext7[4] = {0};
    uint32_t basic_max, ext_max, family, model;
    char vendor[13], hypervisor[13];
    int column = LABEL_WIDTH;

    cpuid(LEAF_VENDOR, 0, regs);
    basic_max = regs[EAX];
    /* The vendor is EBX, EDX, ECX */
    regs_string(vendor, regs[EBX], regs[EDX], regs[ECX]);
    cpuid(LEAF_FEATURES, 0, features);
    if (basic_max >= LEAF_EXT_FEATURES)
        cpuid(LEAF_EXT_FEATURES, 0, ext7);
    cpuid(LEAF_EXT_MAX, 0, regs);
    ext_max = regs[EAX] >= LEAF_EXT_MAX ? regs[EAX] : 0;
    if (ext_max >= LEAF_EXT_FLAGS)
        cpuid(LEAF_EXT_FLAGS, 0, ext);

    field("Architecture", "i686");
    field("CPU op-mode(s)", ext[EDX] & EXT_EDX_LM ? "32-bit, 64-bit" : "32-bit");
    if (ext_max >= LEAF_ADDR_SIZES) {
        char buf[12];

        cpuid(LEAF_ADDR_SIZES, 0, regs);
        label("Address sizes");
        puts_syscall(utoa(regs[EAX] & 0xFF, buf, sizeof(buf)));
        puts_syscall(" bits physical, ");
        puts_syscall(utoa((regs[EAX] >> 8) & 0xFF, buf, sizeof(buf)));
        puts_syscall(" bits virtual\n");
    }
    field("Byte Order", "Little Endian");
    /* Logical processors in the package, butos only runs on the boot one */
    field_num("CPU(s)", features[EDX] & FEAT_EDX_HTT ? (features[EBX] >> 16) & 0xFF : 1, "");
    field("On-line CPU(s) list", "0");
    field("Vendor ID", vendor);
    print_brand(ext_max);

    family = (features[EAX] >> 8) & 0xF;
    model = (features[EAX] >> 4) & 0xF;
    if (family == 0x6 || family == 0xF)
        model += ((features[EAX] >> 16) & 0xF) << 4;
    if (family == 0xF)
        family += (features[EAX] >> 20) & 0xFF;
    field_num("CPU family", family, "");
    field_num("Model", model, "");
    field_num("Stepping", features[EAX] & 0xF, "");

    if (features[ECX] & FEAT_ECX_HYPERVISOR) {
        cpuid(LEAF_HYPERVISOR, 0, regs);
        regs_string(hypervisor, regs[EBX], regs[ECX], regs[EDX]);
        field("Hypervisor vendor", hypervisor[0] ? hypervisor : "unknown");
    }

    if (strcmp(vendor, "GenuineIntel") == 0 && basic_max >= LEAF_CACHE)
        print_caches_intel();
    else
        print_caches_amd(ext_max);

    label("Flags");
    print_flags(flags_1_edx, features[EDX], &column);
    print_flags(flags_ext_edx, ext[EDX], &column);
    print_flags(flags_1_ecx, features[ECX], &column);
    print_flags(flags_7_ebx, ext7[EBX], &column);
    print_flags(flags_ext_ecx, ext[ECX], &column);
    putc_syscall('\n');

    return EXIT_SUCCESS;
}
