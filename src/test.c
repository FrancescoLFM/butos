#include <drivers/ata.h>
#include <drivers/vga.h>
#include <drivers/pci.h>
#include <libs/print.h>
#include <libs/string.h>
#include <libs/allocator.h>
#include <libs/scan.h>
#include <libs/alloc.h>

void test_scan()
{
    const size_t SIZE = 50;
    char *buffer;
    char *delim = " .,;:?!-";

    buffer = kalloc(SIZE * sizeof(*buffer));
    if (!buffer) {
        puts("Failed to allocate buffer\n");
        return;
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
}

void test_allocator()
{
    allocator_t a;
    allocator_t *allocator = &a;

    void *heap_start = (void *)0x200000;
    size_t heap_size = 1000;
    void *registry_start = (char *)heap_start + heap_size;
    size_t registry_size = 100 * sizeof(struct memspace);

    allocator_init(
        allocator,
        heap_start,
        heap_size,
        registry_start,
        registry_size
    );

    uint8_t *ptrs[7];
    for (size_t i = 0; i < 7; i++)
        ptrs[i] = allocator_alloc(allocator, i);
    
    allocator_free(allocator, ptrs[4]);
    ptrs[3][0] = 1;
    ptrs[3][1] = 2;
    ptrs[3][2] = 3;
    ptrs[3][3] = 4;

    for (int i = 0; i < 4; i++)
        printk("%u ", (uint32_t)ptrs[3][i]);
    putc('\n');

    ptrs[3] = allocator_realloc(allocator, ptrs[3], 8);

    /* la presenza di un numero diverso da 4 alla fine dimostra
     * l'avvenuta copiatura
     */
    for (int i = 0; i < 4; i++)
        printk("%u ", (uint32_t)ptrs[3][i]);
    putc('\n');

    allocator_print(allocator, 1);
    puts("- - - - \n");

    for (int i = 0; i < 7; i++)
        if (i != 0 && i != 4) allocator_free(allocator, ptrs[i]);
    allocator_print(allocator, 1);
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

    devs = devs_list_init();
    if (devs == NULL)
        return;
    pci_scan_devices(devs);

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

#define SECTOR_SELECTED 0
#define WORD_PER_ROW    26

void test_ata_io()
{
    uint8_t words_readed = 0;
    struct ata_drive *ata_drive_ptr;
    struct devs_list *devs;

    devs = devs_list_init();
    if (devs == NULL)
        return;
    pci_scan_devices(devs);


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

    devs = devs_list_init();
    if (devs == NULL)
        return;
    pci_scan_devices(devs);

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

