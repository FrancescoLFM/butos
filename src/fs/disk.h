#ifndef DISK_H
#define DISK_H

#include <include/def.h> 

#define MBR_START           0x1BE
#define PART_TYPE_OFFSET    4
#define PART_START_OFFSET   0x08
#define PART_NUM            4
#define MBR_ENTRY_SIZE      16

typedef enum {
    ATA_DRIVE
} disk_type;

typedef enum {
    DISK_SUCCESS,
    DISK_ERROR
} disk_status;

/* Abstract disk interface struct */
struct disk {
    disk_type type;
    uint32_t part_offset;
    void *drive_ptr;
};

disk_status disk_read(struct disk *disk, uint8_t n, uint32_t lba, uint8_t *buff);
disk_status disk_write(struct disk *disk, uint8_t n, uint32_t lba, uint8_t *buff);
struct disk *disk_init(disk_type type);
disk_status disk_set_offset(struct disk *disk, uint8_t part_type);
void disk_fini(struct disk *disk);

#endif