#ifndef DISK_H
#define DISK_H

#include <include/def.h> 

#define MBR_START           0x1BE
#define PART_TYPE_OFFSET    4
#define PART_START_OFFSET   0x08
#define PART_NUM            4
#define MBR_ENTRY_SIZE      16

typedef enum DISK_TYPE_T {
    ATA_DRIVE
} DISK_TYPE;

/* Abstract disk interface struct */
struct disk {
    DISK_TYPE type;
    uint32_t part_offset;
    void *drive_ptr;
};

void disk_read(struct disk *disk, uint8_t n, uint32_t lba, uint8_t *buff);
void disk_write(struct disk *disk, uint8_t n, uint32_t lba, uint8_t *buff);
struct disk *disk_init(DISK_TYPE type);
void disk_set_offset(struct disk *disk, uint8_t part_type);
void disk_fini(struct disk *disk);

#endif