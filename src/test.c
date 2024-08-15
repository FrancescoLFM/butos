#include <drivers/ata.h>
#include <drivers/vga.h>
#include <drivers/pci.h>
#include <libs/print.h>
#include <libs/string.h>
#include <libs/allocator.h>
#include <libs/scan.h>
#include <libs/alloc.h>
#include <fs/disk.h>
#include <fs/fat.h>
#include <libs/syscalls.h>
#include <cpu/paging.h>
#include <fs/elf.h>

#define MEMSPACE_SIZE(START, END)   ((END) - (START) + 1)

void test_allocator()
{
    struct allocator a;
    struct memspace regs[10];
    allocator_init(
        &a,
        (uintptr_t) 0,
        MEMSPACE_SIZE(0, 31),
        (void *) regs,
        sizeof(regs),
        8
    );

    allocator_alloc(&a, 3);
    allocator_alloc(&a, 3);
    allocator_free(&a, 0);
    allocator_alloc(&a, 9);
    allocator_print(&a, 1);
    
}

int test_scan()
{
    const size_t SIZE = 50;
    char *buffer;
    char *delim = " .,;:?!-";

    buffer = kalloc(SIZE * sizeof(*buffer));
    if (!buffer) {
        puts("Failed to allocate buffer\n");
        return 1;
    }

    for (char *tok;;) {
        gets(buffer, SIZE);
        tok = strtok(buffer, delim);
        if (!tok)
            continue;

        puts_c(GREEN, tok);
        puts("\n");
        while ((tok = strtok(NULL, delim))) {
            puts_c(GREEN, tok);
            puts("\n");
        }
    }
    return 0;
}

void test_string()
{
    int buffer[70] = {0};

    for (int i = 0; i < 70; i++)
        printk("%x ", buffer[i]);
    
    memset(buffer, 19, sizeof(buffer));

    for (int i = 0; i < 70; i++)
        printk("%x ", buffer[i]);
}


void test_ata()
{
    struct ata_drive *ata_drive_ptr;
    struct devs_list *devs;

    devs = devs_list_init(5);
    if (devs == NULL)
        return;
    pci_scan_devices(devs, 5);

    ata_drive_ptr = NULL;
    for (size_t n=0; n < devs->len; n++)
        if (devs->buffer[n]->class_code == IDE_CLASSCODE)
            ata_drive_ptr = ata_drive_init(0, 0, devs->buffer[n]);
    
    if (ata_drive_ptr == NULL) {
        devs_list_destroy(devs);
        return;
    }

    devs_list_destroy(devs);
    ata_drive_fini(ata_drive_ptr);
}

#define SECTOR_SELECTED 203
#define WORD_PER_ROW    26

void test_ata_io()
{
    uint8_t words_readed = 0;
    struct ata_drive *ata_drive_ptr;
    struct devs_list *devs;
    size_t len;

    len = pci_get_device_num();
    devs = devs_list_init(len);
    if (devs == NULL)
        return;
    pci_scan_devices(devs, len);

    uint8_t *sector_buff = kalloc(ATA_SECTOR_SIZE);
    if (sector_buff == NULL) {
        devs_list_destroy(devs);
        return;
    }
    for (int i=0; i < ATA_SECTOR_SIZE; i++)
        sector_buff[i] = 0;

    ata_drive_ptr = NULL;
    for (size_t n=0; n < devs->len; n++)
        if (devs->buffer[n]->class_code == IDE_CLASSCODE)
            ata_drive_ptr = ata_drive_init(0, 0, devs->buffer[n]);
    
    if (ata_drive_ptr == NULL) {
        devs_list_destroy(devs);
        return;
    }

    // strcpy((char *)sector_buff, "Buon natale da butos");

    // ata_drive_write_pio(ata_drive_ptr, 1, SECTOR_SELECTED, sector_buff);

    ata_drive_read_pio(ata_drive_ptr, 1, SECTOR_SELECTED, sector_buff);

    printk("Sector %d: \n", SECTOR_SELECTED);

    for (uint32_t i=0; i < ATA_SECTOR_SIZE; i++) {
        printk("%x ", sector_buff[i]);
        
        if (++words_readed == WORD_PER_ROW) {
            words_readed = 0;
            putc('\n');

        }
    }

    putc('\n');
    ata_drive_fini(ata_drive_ptr);
    devs_list_destroy(devs);
    kfree(sector_buff);
}

void test_pci()
{
    struct devs_list *devs;

    devs = devs_list_init(5);
    if (devs == NULL)
        return;
    pci_scan_devices(devs, 5);

    for (size_t i = 0; i < devs->len; i++) {
            if (devs->buffer[i]->device_id != 0xFFFF) {
                printk("ID: %d %s: %s\n", devs->buffer[i]->device_id,
                       pci_get_class(devs->buffer[i]->class_code), 
                       pci_get_subclass(devs->buffer[i]->class_code, devs->buffer[i]->subclass));
            }
        }
}

void test_ide_controller()
{
    struct pci_device_info dev_info;
    struct pci_device_info *dev_info_ptr = &dev_info;

    for (uint16_t bus = 0; bus < 256; bus++)
        for (uint8_t dev = 0; dev < 32; dev++) {
            pci_read_device_info(bus, dev, dev_info_ptr);
            if (dev_info_ptr->class_code == IDE_CLASSCODE) 
            {
                printk("BAR0: 0x%x\n", BAR0(bus, dev));
                printk("BAR1: 0x%x\n", BAR1(bus, dev));
                printk("BAR2: 0x%x\n", BAR2(bus, dev));
                printk("BAR3: 0x%x\n", BAR3(bus, dev));
                printk("BAR4: 0x%x\n", BAR4(bus, dev));
                printk("PROGIF: 0x%x\n", dev_info_ptr->prog_if);
                printk("VENDOR ID: 0x%x\n", dev_info_ptr->vendor_id);
                printk("DEVICE ID: 0x%x\n", dev_info_ptr->device_id);
                printk("HEADER TYPE: 0x%x\n", dev_info_ptr->header_type);
                printk("INTERRUPT LINE: %d\n", dev_info_ptr->int_line);
                printk("INTERRUPT PIN: 0x%x\n", dev_info_ptr->int_PIN);
            }
        }
}

void disk_test() {
    struct disk *disk;
    uint8_t first_sector[512];
    
    disk = disk_init(ATA_DRIVE);
    if (disk == NULL)
        return;

    puts("Disk initialized!\n");
    printk("Disk type 0x%x\n", disk->type);
    disk_set_offset(disk, FAT_PART_TYPE);
    printk("Disk part offset 0x%x\n", disk->part_offset);
    disk_read(disk, 1, 0, first_sector);

    uint8_t words_readed = 0;

    for (uint32_t i=0; i < ATA_SECTOR_SIZE; i++) {
        printk("%x ", first_sector[i]);
        
        if (++words_readed == WORD_PER_ROW) {
            words_readed = 0;
            putc('\n');

        }
    }
}

void fs_test() {
    struct disk *disk;
    fat_fs_t *fs;
    file_t *file;

    disk = disk_init(ATA_DRIVE);
    if (disk == NULL)
        return;
    disk_set_offset(disk, FAT_PART_TYPE);
    
    fs = fat_fs_init(disk);
    if (fs == NULL)
         return;
    fat_fs_printinfo(fs);

    if(file_create(fs, "/", "prova.txt")) {
        puts("FILESYSTEM TEST 0: NOT PASSED\n");
        return;
    } else {
        puts("FILESYSTEM TEST 0: PASSED\n"); 
    }
    file = file_open_path(fs, "/prova.txt");
    if (file == NULL) {
        puts("FILESYSTEM TEST 1: NOT PASSED\n");
        return;
    } else {
        puts("FILESYSTEM TEST 1: PASSED\n");
    }

    char *string_test = "This is a test";
    file_write(file, fs, 0, (uint8_t *) string_test, 15);
    uint8_t *read = file_read(file, fs, 0, 15);
    puts((char *)read);
    putc('\n');
    if (strncmp(string_test, (char *) read, 15))
        puts("FILESYSTEM TEST 2: NOT PASSED\n");
    else
        puts("FILESYSTEM TEST 2: PASSED\n");
    
    kfree(read);
    file_close(fs, file);
    fat_fs_fini(fs);
    disk_fini(disk);

    kalloc_print();
}

void syscall_test() {
    void *buff = NULL;
    char c = 0;
    kalloc_syscall(&buff, 32);
    printf_syscall("Buffer: 0x%x\n", (uintptr_t) &buff);
    kfree_syscall(buff);
    kalloc_print();
    getchar_syscall(&c);
    clear_syscall(BLACK);
}

void elf_test() {
    elf_t elf;
    struct disk *disk;
    fat_fs_t *fs;
    file_t *file;

    disk = disk_init(ATA_DRIVE);
    if (disk == NULL)
        return;
    disk_set_offset(disk, FAT_PART_TYPE);
    
    fs = fat_fs_init(disk);
    if (fs == NULL)
         return;

    file = file_open_path(fs, "/example");
    if (fs == NULL) {
        puts("No program named example");
        return;
    }
    if (elf_init(&elf, file, fs))
        return;
    
    puts("Success");
    file_close(fs, file);
    fat_fs_fini(fs);
    disk_fini(disk);
}