#include <fs/blk.h>
#include <fs/fd.h>
#include <drivers/ata.h>
#include <libs/string.h>

#define MBR_BOOT_FLAG       0x80
#define PART_SIZE_OFFSET    0x0C
#define LE32(BUF, OFF)      ((uint32_t) (BUF)[OFF] | ((uint32_t) (BUF)[(OFF) + 1] << 8) | \
                             ((uint32_t) (BUF)[(OFF) + 2] << 16) | ((uint32_t) (BUF)[(OFF) + 3] << 24))

static struct disk *blk_disk;

void blk_init(struct disk *disk)
{
    blk_disk = disk;
}

static void blk_disk_info(struct ata_drive *drive, blk_info_t *out)
{
    size_t len;

    strcpy(out->name, "hda");
    strncpy(out->model, drive->ident->model, BLK_MODEL_LEN - 1);
    /* IDENTIFY pads the model with spaces */
    for (len = strlen(out->model); len > 0 && out->model[len - 1] == ' '; len--)
        out->model[len - 1] = '\0';
    out->sectors = drive->ident->max_lba;
}

int blk_info(int index, blk_info_t *out)
{
    uint8_t mbr[ATA_SECTOR_SIZE];
    struct ata_drive *drive;
    uint8_t *part;

    if (blk_disk == NULL || out == NULL || index < 0)
        return FD_INVALID;
    if (blk_disk->type != ATA_DRIVE)
        return 0;
    drive = (struct ata_drive *) blk_disk->drive_ptr;

    memset(out, 0, sizeof(*out));
    if (index == 0) {
        blk_disk_info(drive, out);
        return 1;
    }

    /* Read the MBR directly, disk_read would add the partition offset */
    if (ata_drive_read_pio(drive, 1, 0, mbr))
        return FD_IO_ERROR;
    for (int i = 0; i < PART_NUM; i++) {
        part = mbr + MBR_START + i * MBR_ENTRY_SIZE;
        if (part[PART_TYPE_OFFSET] == 0 || --index > 0)
            continue;
        strcpy(out->name, "hda");
        out->name[3] = (char) ('1' + i);
        out->part_type = part[PART_TYPE_OFFSET];
        out->bootable = part[0] == MBR_BOOT_FLAG;
        out->start = LE32(part, PART_START_OFFSET);
        out->sectors = LE32(part, PART_SIZE_OFFSET);
        if (out->start == blk_disk->part_offset)
            strcpy(out->mount, "/");
        return 1;
    }

    return 0;
}
