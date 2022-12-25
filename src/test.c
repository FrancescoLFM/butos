#include <drivers/ata.h>
#include <libs/print.h>
#include <libs/string.h>

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

    for (uint32_t i=(ATA_SECTOR_SIZE * (SECTOR_SELECTED - 1)); i < ATA_SECTOR_SIZE * SECTOR_SELECTED; i++) {
        putc(STD_COLOR, sector_buff[i]);
        
        if (++words_readed == WORD_PER_ROW) {
            words_readed = 0;
            putc(STD_COLOR, '\n');
        };
    }

    putc(STD_COLOR, '\n');

}