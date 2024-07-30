#include <fs/disk.h>
#include <libs/alloc.h>
#include <drivers/ata.h>
#include <drivers/pci.h>
#include <fs/fat.h>

void disk_read(struct disk *disk, uint8_t n, uint32_t lba, uint8_t *buff) {
    switch (disk->type) {
        case ATA_DRIVE:
        ata_drive_read_pio((struct ata_drive *) disk->drive_ptr, n, lba + disk->part_offset, buff);
        break;
    }
}

void disk_write(struct disk *disk, uint8_t n, uint32_t lba, uint8_t *buff) {
    switch (disk->type) {
        case ATA_DRIVE:
        ata_drive_write_pio((struct ata_drive *) disk->drive_ptr, n, lba + disk->part_offset, buff);
        break;
    }
}

void disk_set_offset(struct disk *disk, uint8_t part_type) {
    uint8_t first_sector[ATA_SECTOR_SIZE];

    disk_read(disk, 1, 0, first_sector);
    for (int i = 0; i < PART_NUM; i++) {
        int part_entry_start = MBR_START + (i * MBR_ENTRY_SIZE);
        if (first_sector[part_entry_start + PART_TYPE_OFFSET] == part_type) {
            disk->part_offset = BYTES_TO_LONG(first_sector, part_entry_start + PART_START_OFFSET);
            return;
        }
    }
}

struct disk *disk_init(DISK_TYPE type) {
    size_t len;
    struct devs_list *devs;
    struct disk *disk = kalloc(sizeof(*disk));
    if (disk == NULL)
        return NULL;

    disk->type = type;
    switch (type) {
        case ATA_DRIVE:
        len = pci_get_device_num();
        devs = devs_list_init(len);
        if (devs == NULL)
            return NULL;
        pci_scan_devices(devs, len);
        disk->drive_ptr = NULL;
        for (size_t n=0; n < devs->len; n++)
            if (devs->buffer[n]->class_code == IDE_CLASSCODE)
                disk->drive_ptr = (void *) ata_drive_init(0, 0, devs->buffer[n]);
        
        if (disk->drive_ptr == NULL) {
            devs_list_destroy(devs);
            kfree(disk);
            return NULL;
        }

        disk->part_offset = 0;

        break;
    }
    return disk;
}

void disk_fini(struct disk *disk)
{
    switch (disk->type) {
        case ATA_DRIVE:
        ata_drive_fini((struct ata_drive *)disk->drive_ptr);
        break;
    }
    kfree(disk);
}