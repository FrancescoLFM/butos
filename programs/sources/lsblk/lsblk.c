#include <include/syscalls.h>
#include <libs/string.h>

#define SECTOR_SIZE     512

/* Column widths, the last column (MODEL) isn't padded */
#define W_NAME          9
#define W_SIZE          8
#define W_TYPE          6
#define W_FSTYPE        7
#define W_START         10
#define W_SECTORS       10
#define W_MOUNT         11

static void pad(const char *s, int width)
{
    int len = (int) strlen(s);

    puts_syscall((char *) s);
    for (; len < width; len++)
        putc_syscall(' ');
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

/* Human readable size with one decimal, like 31.5M */
static char *human_size(unsigned int sectors, char *buf)
{
    const char units[] = "KMGT";
    unsigned int kib = sectors / (1024 / SECTOR_SIZE);
    unsigned int div = 1;
    char digits[12];
    size_t u = 0;

    /* The tenths are computed as (kib % div) * 10 / div: div stays at most 1024^2 to not overflow */
    while (kib / div >= 1024 && u < 2) {
        div *= 1024;
        u++;
    }
    strcpy(buf, utoa(kib / div, digits, sizeof(digits)));
    if (u > 0 && kib / div < 100) {
        strcat(buf, ".");
        strcat(buf, utoa((kib % div) * 10 / div, digits, sizeof(digits)));
    }
    buf[strlen(buf) + 1] = '\0';
    buf[strlen(buf)] = units[u];

    return buf;
}

static const char *fstype(unsigned int part_type)
{
    switch (part_type) {
        case 0x01:                          return "fat12";
        case 0x04: case 0x06: case 0x0E:    return "fat16";
        case 0x0B: case 0x0C:               return "vfat";
        case 0x07:                          return "ntfs";
        case 0x82:                          return "swap";
        case 0x83:                          return "linux";
        case 0x05: case 0x0F:               return "extended";
        case 0xEE:                          return "gpt";
        default:                            return "?";
    }
}

int main(void)
{
    struct blkinfo info;
    char num[12];
    char size[16];
    char name[12];
    int ret;

    pad("NAME", W_NAME);
    pad("SIZE", W_SIZE);
    pad("TYPE", W_TYPE);
    pad("FSTYPE", W_FSTYPE);
    pad("START", W_START);
    pad("SECTORS", W_SECTORS);
    pad("MOUNTPOINT", W_MOUNT);
    puts_syscall("MODEL\n");

    for (int i = 0; (ret = blkinfo_syscall(i, &info)) > 0; i++) {
        strcpy(name, info.part_type ? "`-" : "");
        strcat(name, info.name);
        pad(name, W_NAME);
        pad(human_size(info.sectors, size), W_SIZE);
        pad(info.part_type ? "part" : "disk", W_TYPE);
        pad(info.part_type ? fstype(info.part_type) : "", W_FSTYPE);
        pad(info.part_type ? utoa(info.start, num, sizeof(num)) : "", W_START);
        pad(utoa(info.sectors, num, sizeof(num)), W_SECTORS);
        if (info.part_type) {
            puts_syscall(info.mount);
        } else {
            pad(info.mount, W_MOUNT);
            puts_syscall(info.model);
        }
        putc_syscall('\n');
    }

    if (ret < 0) {
        puts_syscall("lsblk: can't read the block devices\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
