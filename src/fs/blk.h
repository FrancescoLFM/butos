#ifndef BLK_H
#define BLK_H

#include <include/def.h>
#include <fs/disk.h>

/* Block devices for lsblk: the boot disk ("hda") followed by its MBR partitions ("hda1"...) */

#define BLK_NAME_LEN        8
#define BLK_MODEL_LEN       40
#define BLK_MOUNT_LEN       16

/* Same layout as struct blkinfo in programs/include/syscalls.h */
typedef struct {
    char     name[BLK_NAME_LEN];
    char     model[BLK_MODEL_LEN];  /* Empty for partitions */
    char     mount[BLK_MOUNT_LEN];  /* Empty if not mounted */
    uint32_t start;                 /* First LBA */
    uint32_t sectors;               /* 512 bytes sectors */
    uint32_t part_type;             /* MBR partition type, 0 for the whole disk */
    uint32_t bootable;
} blk_info_t;

/* disk is the boot disk, its partition at disk->part_offset is mounted on / */
void blk_init(struct disk *disk);
/* Returns 1 and fills *out if device index exists, 0 past the last one, negative fd_status_t on error */
int blk_info(int index, blk_info_t *out);

#endif
