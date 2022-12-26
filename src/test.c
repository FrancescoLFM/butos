#include <drivers/ata.h>
#include <libs/print.h>
#include <libs/string.h>

#define BUTOS_SECTORS   0x1e
#define SECTOR_SELECTED 3
#define WORD_PER_ROW    15

void test_string()
{
    int buffer[70] = {0};

    for (int i = 0; i < 70; i++)
        printk(STD_COLOR, "%x ", buffer[i]);
    
    memset(buffer, 19, sizeof(buffer));

    for (int i = 0; i < 70; i++)
        printk(BLUE | GREEN, "%x ", buffer[i]);
}

void test_ata()
{
    uint8_t words_readed = 0;
    struct ata_drive ata_drive_dummy;
    struct ata_drive *ata_drive_ptr = &ata_drive_dummy;

    uint16_t *sector_buff = (uint16_t *)(0x2000000);
    for (int i=0; i < BUTOS_SECTORS * ATA_SECTOR_SIZE; i++)
        sector_buff[i] = 0;

    ata_drive_init(ata_drive_ptr, 0, 0);

    //sector_buff[1] = 'C';

    //ata_drive_access_pio(ata_drive_ptr, ATA_WRITE, 1, SECTOR_SELECTED, sector_buff);

    ata_drive_print_status(ata_drive_ptr);

    ata_drive_read_pio(ata_drive_ptr, BUTOS_SECTORS, 1, sector_buff);

    printk(STD_COLOR, "Sector %d: \n", SECTOR_SELECTED);

    for (uint32_t i=(ATA_SECTOR_SIZE * (SECTOR_SELECTED - 1)); i < ATA_SECTOR_SIZE * SECTOR_SELECTED; i++) {
        printk(STD_COLOR, "%x ", sector_buff[i]);
        
        if (++words_readed == WORD_PER_ROW) {
            words_readed = 0;
            putc(STD_COLOR, '\n');
        };
    }

    putc(STD_COLOR, '\n');

    ata_drive_print_status(ata_drive_ptr);

}