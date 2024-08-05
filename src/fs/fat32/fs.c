#include <fs/fat.h>
#include <libs/alloc.h>
#include <libs/string.h>
#include <fs/disk.h>
#include <libs/print.h>

fat_volume_t *fat_volume_init(struct disk *drive)
{
    fat_volume_t *volume;

    volume = kalloc(sizeof(*volume));
    if (volume == NULL) {
        puts("Malloc error: not enough space to allocate volume struct");
        return NULL;
    }
    volume->drive = drive;
    volume->label = kalloc(sizeof(char) * LABEL_LENGTH);
    if (volume->label == NULL) {
        puts("Malloc error: not enough space to allocate volume label");
        kfree(volume);
        return NULL;
    }
    volume->sector_count = UNDEFINED_SECCOUNT;
    volume->sector_size = SECTOR_SIZE;

    return volume;
}

void fat_volume_fini(fat_volume_t *volume)
{
    kfree(volume->label);
    kfree(volume);

    return;
}

void fat_volume_getinfo(fat_volume_t *volume, uint8_t *info_buffer)
{
    volume->sector_size = BYTES_TO_WORD(info_buffer, BYTES_PER_SECTOR);

    if (info_buffer[SECTOR_COUNT] != 0)
        volume->sector_count = BYTES_TO_WORD(info_buffer, SECTOR_COUNT);
    else
        volume->sector_count = BYTES_TO_LONG(info_buffer, LARGE_SECTOR_COUNT);

    volume->cluster_size = info_buffer[SECTOR_PER_CLUSTER];
    volume->cluster_sizeb = volume->cluster_size * volume->sector_size;
}

void fat_table_getinfo(fat_table_t *fat_table, uint8_t *info_buffer)
{
    fat_table->address = BYTES_TO_WORD(info_buffer, RESERVED_SECTORS);
    fat_table->size = BYTES_TO_LONG(info_buffer, FAT_TABLE_SIZE);
    fat_table->count = info_buffer[FAT_TABLE_COUNT];
}

void fat_volume_get_clusters(fat_volume_t *volume, fat_table_t *table)
{
    volume->cluster_count = (volume->sector_count - table->address -
                             (table->size * table->count)) /
                            volume->cluster_size;
}

uint8_t fat_fs_getinfo(fat_fs_t *fs)
{
    uint8_t *info_buffer;
    uint8_t err = 0;

    info_buffer = read_sector(fs, BPB_SECTOR);
    if (info_buffer == NULL)
        return FS_ERROR;

    fs->info.sector = BYTES_TO_WORD(info_buffer, FSINFO_SECTOR);
    fs->info.buffer = read_sector(fs, fs->info.sector);
    if (fs->info.buffer  == NULL ||
        BYTES_TO_LONG(fs->info.buffer , LEAD_SIGNATURE1_OFF) != LEAD_SIGNATURE1 ||
        BYTES_TO_LONG(fs->info.buffer , LEAD_SIGNATURE2_OFF) != LEAD_SIGNATURE2)
    {
        err = FS_ERROR;
        goto exit;
    }

    fat_volume_getinfo(fs->volume, info_buffer);
    fat_table_getinfo(fs->table, info_buffer);

    fat_volume_get_clusters(fs->volume, fs->table);

    fs->info.root_cluster = BYTES_TO_LONG(info_buffer, ROOT_CLUSTER);
    fs->info.free_cluster_count = BYTES_TO_LONG(fs->info.buffer , FREE_CLUSTER_COUNT);
    if (fs->info.free_cluster_count == UNKNOWN_FREE_CLUSTER)
        fs->info.free_cluster_count = free_cluster_count_read(fs);
    fs->info.free_cluster = BYTES_TO_LONG(fs->info.buffer , FREE_CLUSTER);
    if (fs->info.free_cluster == UNKNOWN_FREE_CLUSTER)
        fs->info.free_cluster = first_free_cluster_read(fs);
    fs->info.data_region = (fs->table->size * fs->table->count) + fs->table->address;

exit:
    kfree(info_buffer);

    return err;
}

void fat_fsinfo_flush(fat_fs_t *fs)
{
    memcpy(fs->info.buffer + FREE_CLUSTER_COUNT, 
          &fs->info.free_cluster_count, sizeof(fs->info.free_cluster_count));
    memcpy(fs->info.buffer + FREE_CLUSTER, 
          &fs->info.free_cluster, sizeof(fs->info.free_cluster));

    write_sector(fs, fs->info.sector, fs->info.buffer);
}

entry_t *fake_entry_create(uint32_t cluster, char *name, size_t size)
{
    entry_t *fake_entry;

    fake_entry = kcalloc(1, sizeof(*fake_entry));
    if (fake_entry == NULL) {
        puts("Malloc error: not enough space to allocate fake entry");
        return NULL;
    }

    strncpy(fake_entry->short_name, name, SHORT_NAME_LEN);
    fake_entry->low_cluster = cluster & 0xFF;
    fake_entry->high_cluster = cluster >> 16;
    fake_entry->size = size;

    return fake_entry;
}

dir_t *fat_root_dir_init(fat_fs_t *fs)
{
    entry_t *root_entry;
    dir_t *root_dir;

    root_entry = fake_entry_create(fs->info.root_cluster, "/", fs->volume->sector_count * fs->volume->sector_size);
    if (root_entry == NULL)
        return NULL;
    root_dir = dir_init(fs, root_entry);
    if (root_dir == NULL) {
        kfree(root_entry);
        return NULL;
    }

    kfree(root_entry);
    return root_dir;
}

fat_fs_t *fat_fs_init(struct disk *partition)
{
    fat_fs_t *fs;

    fs = kcalloc(1, sizeof(*fs));
    if (fs == NULL) {
        puts("Malloc error: not enough space to allocate filesystem\n");
        return NULL;
    }
    fs->root_dir = NULL;

    fs->volume = fat_volume_init(partition);
    if (fs->volume == NULL) {
        kfree(fs);
        return NULL;
    }
    
    fs->table = fat_table_init(fs->volume);
    if (fs->table == NULL) {
        fat_volume_fini(fs->volume);
        kfree(fs);
        return NULL;
    }

    if (fat_fs_getinfo(fs) == FS_ERROR) {
        puts("Fsinfo error: filesystem info structure is corrupted\n");
        fat_fs_fini(fs);
        return NULL;
    }

    fs->root_dir = fat_root_dir_init(fs);
    if (fs->root_dir == NULL) {
        puts("Malloc error: not enough space to allocate root directory\n");
        fat_fs_fini(fs);
        return NULL;
    }
    return fs;
}

void fat_fs_fini(fat_fs_t *fs)
{
    if (fs->root_dir != NULL)
        dir_close(fs, fs->root_dir);
    
    if (fs->info.buffer != NULL) {
        fat_fsinfo_flush(fs);
        kfree(fs->info.buffer);
    }
    
    fat_table_fini(fs->table, fs);
    fat_volume_fini(fs->volume);
    kfree(fs);
}


uint8_t *read_sectors(fat_fs_t *fs, uint32_t lba, uint32_t n)
{
    uint8_t *buffer;
    fat_volume_t *volume = fs->volume;

    if (lba + n - 1 > volume->sector_count) {
        puts("Disk error: tried reading off the bounds.");
        return NULL;
    }

    buffer = kalloc(sizeof(uint8_t) * volume->sector_size * n);
    if (buffer == NULL) {
        puts("Malloc error: not enough space to allocate disk buffer");
        return NULL;
    }
    
    disk_read(volume->drive, n, lba, buffer);

    return buffer;
}

uint8_t *read_sector(fat_fs_t *fs, uint32_t lba)
{
    return read_sectors(fs, lba, 1);
}

uint8_t write_sectors(fat_fs_t *fs, uint32_t lba, uint8_t *buffer, uint32_t n)
{
    fat_volume_t *volume = fs->volume;

    if (lba + n - 1 > volume->sector_count) {
        puts("Disk error: tried writing off the bounds.");
        return EXIT_FAILURE;
    }

    disk_write(volume->drive, n, lba, buffer);
    return EXIT_SUCCESS;
}

uint8_t write_sector(fat_fs_t *fs, uint32_t lba, uint8_t *buffer)
{
    return write_sectors(fs, lba, buffer, 1);
}

uint8_t *read_cluster(fat_fs_t *fs, uint32_t cluster)
{
    uint32_t offset;
    uint8_t *buffer;

    if (cluster > fs->volume->cluster_count) {
        puts("Disk error: reading cluster off the bounds\n");
        printk("Cluster: %d\n", cluster);
        return NULL;
    }

    offset = cluster * fs->volume->cluster_size;

    buffer = read_sectors(fs, fs->info.data_region + offset - 2, fs->volume->cluster_size);
    if (buffer == NULL)
        return NULL;
    
    return buffer;
}

uint8_t write_cluster(fat_fs_t *fs, uint32_t cluster, uint8_t *buffer)
{
    uint32_t offset;

    if (cluster > fs->volume->cluster_count) {
        puts("Disk error: writing cluster off the bounds\n");
        printk("Cluster: %d\n", cluster);
        return EXIT_FAILURE;
    }

    offset = cluster * fs->volume->cluster_size;
    
    write_sectors(fs, fs->info.data_region + offset - 2, buffer, fs->volume->cluster_size);
    return EXIT_SUCCESS;
}

void fat_fs_printinfo(fat_fs_t *fs)
{
    puts("-------------------------------\n");
    printk("Volume label:\t\t%s\n", fs->volume->label);
    printk("Sector count:\t\t%u\n", fs->volume->sector_count);
    printk("Sector size:\t\t%uB\n", fs->volume->sector_size);
    printk("Cluster size:\t\t%u Sec\n", fs->volume->cluster_size);
    printk("Cluster count:\t\t%u\n\n", fs->volume->cluster_count);
    printk("Fat table address:\t\t%u\n", fs->table->address);
    printk("Fat table size:\t\t%u Sec\n", fs->table->size);
    printk("Fat table count:\t\t%u FATs\n", fs->table->count);
    if (fs->table->cache != NULL)
        puts("Fat table cache ENGAGED!\n");
    printk("Root cluster:\t\t%u\n", fs->info.root_cluster);
    printk("Free cluster count:\t\t%u\n", fs->info.free_cluster_count);
    printk("First free cluster:\t\t%u\n", fs->info.free_cluster);
    printk("Data region starts: \t0x%x\n", fs->info.data_region * 512); 
    puts("-------------------------------\n");
}

