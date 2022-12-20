#include <drivers/ata.h>
#include <libs/print.h>
#include <libs/string.h>

#define BUTOS_SECTORS   0x1d
#define SECTOR_SELECTED 0x1d

void test_ata()
{
    struct ata_drive ata_drive_dummy;
    struct ata_drive *ata_drive_ptr = &ata_drive_dummy;

    uint16_t *sector_buff = (uint16_t *)(0x2000000);
    for (int i=0; i < BUTOS_SECTORS * ATA_SECTOR_SIZE; i++)
        sector_buff[i] = 0;

    ata_drive_init(ata_drive_ptr, 0, 0);

    ata_drive_read_pio(ata_drive_ptr, BUTOS_SECTORS, 1, sector_buff);

    printk(STD_COLOR, "Sector %d: \n", SECTOR_SELECTED);

    for (uint32_t i=(ATA_SECTOR_SIZE * (SECTOR_SELECTED - 1)); i < ATA_SECTOR_SIZE * SECTOR_SELECTED; i++)
        printk(STD_COLOR, "%x ", sector_buff[i]);
}