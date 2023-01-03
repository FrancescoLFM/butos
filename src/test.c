#include <drivers/ata.h>
#include <drivers/vga.h>
#include <libs/print.h>
#include <libs/string.h>
#include <libs/allocator.h>
#include <libs/scan.h>
#include <libs/alloc.h>

void test_scan()
{
    const size_t SIZE = 50;
    char *buffer;

    buffer = kalloc(SIZE * sizeof(*buffer));
    if (!buffer)
        puts("Failed to allocate buffer\n");

    for (;;) {
        gets(buffer, SIZE);
        print_pm(RED, buffer);
    }
}

void test_allocator()
{
    allocator_t a;
    allocator_t *allocator = &a;

    void *heap_start = (void *)0x200000;
    size_t heap_size = 1000;
    void *registry_start = heap_start + heap_size;
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
        printk(STD_COLOR, "%u ", (uint32_t)ptrs[3][i]);
    putc(STD_COLOR, '\n');

    ptrs[3] = allocator_realloc(allocator, ptrs[3], 8);

    /* la presenza di un numero diverso da 4 alla fine dimostra
     * l'avvenuta copiatura
     */
    for (int i = 0; i < 4; i++)
        printk(STD_COLOR, "%u ", (uint32_t)ptrs[3][i]);
    putc(STD_COLOR, '\n');

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
        printk(STD_COLOR, "%x ", buffer[i]);
    
    memset(buffer, 19, sizeof(buffer));

    for (int i = 0; i < 70; i++)
        printk(BLUE | GREEN, "%x ", buffer[i]);
}


#define BUTOS_SECTORS   0x1d
#define SECTOR_SELECTED 0x1f
#define WORD_PER_ROW    23

void test_ata()
{
    uint8_t words_readed = 0;
    struct ata_drive ata_drive_dummy;
    struct ata_drive *ata_drive_ptr = &ata_drive_dummy;

    uint8_t *sector_buff = (uint8_t *)(0x2000000);
    for (int i=0; i < BUTOS_SECTORS * ATA_SECTOR_SIZE; i++)
        sector_buff[i] = 0;

    ata_drive_init(ata_drive_ptr, 0, 0);

    strcpy((char *)sector_buff, "Buon natale da butos");

    ata_drive_write_pio(ata_drive_ptr, 1, SECTOR_SELECTED, sector_buff);

    ata_drive_read_pio(ata_drive_ptr, SECTOR_SELECTED, 1, sector_buff);

    printk(STD_COLOR, "Sector %d: \n", SECTOR_SELECTED);

    for (
        uint32_t i=(ATA_SECTOR_SIZE * (SECTOR_SELECTED - 1));
        i < ATA_SECTOR_SIZE * SECTOR_SELECTED;
        i++
    ) {
        putc(STD_COLOR, sector_buff[i]);
        
        if (++words_readed == WORD_PER_ROW) {
            words_readed = 0;
            putc(STD_COLOR, '\n');

        }
    }

    putc(STD_COLOR, '\n');

}
