#ifndef FAT_H
#define FAT_H

#include <include/def.h>
#include <fs/disk.h>

#define CACHE_READ      0
#define CACHE_WRITE     1

#define LABEL_LENGTH                 11
#define SECTOR_SIZE                  512
#define UNDEFINED_SECCOUNT           65535

// Macros
#define BYTES_TO_WORD(BYTES, OFF)    (BYTES[0 + OFF] + ((uint16_t) (BYTES[1 + OFF]) << 8))
#define BYTES_TO_LONG(BYTES, OFF)    (BYTES[0 + OFF] + (BYTES[1 + OFF] << 8) + (BYTES[2 + OFF] << 16) + (BYTES[3 + OFF] << 24))

#define BPB_SECTOR              0

#define BYTES_PER_SECTOR        0x0B
#define SECTOR_PER_CLUSTER      0x0D
#define RESERVED_SECTORS        0x0E
#define FAT_TABLE_COUNT         0x10
#define SECTOR_COUNT            0x13
#define LARGE_SECTOR_COUNT      0x20
#define FAT_TABLE_SIZE          0x24
#define ROOT_CLUSTER            0x2C
#define FSINFO_SECTOR           0x30

#define LEAD_SIGNATURE1_OFF     0x00
#define LEAD_SIGNATURE1         0x41615252
#define LEAD_SIGNATURE2_OFF     0x1E4
#define LEAD_SIGNATURE2         0x61417272
#define FREE_CLUSTER_COUNT      0x1E8
#define FREE_CLUSTER            0x1EC
#define UNKNOWN_FREE_CLUSTER    0xFFFFFFFF

#define FAT_READ                0
#define FAT_WRITE               1

#define FS_ERROR                1
#define READ_ERROR              0xFFFFFFFF
#define CLUSTER_ALLOC_ERR       1

#define EOC1                    0xFFFFFF8
#define EOC2                    0xFFFFFFF
#define FAT_EOF                 0x1a
#define SHORT_NAME_LEN          11
#define FILENAME_LEN            8
#define FILE_EXT_LEN            3
#define WORDS_TO_LONG(HIGH, LOW)    (LOW + (HIGH << 16))

#define LFN_ATTR                0x0F
#define DIR_ATTR                0x10
#define FILE_ATTR               0x20
#define ATTR_OFFSET             11
#define CTIME_OFFSET            14
#define ATIME_OFFSET            18
#define HIGH_CLUSTER_OFFSET     20
#define MTIME_OFFSET            22
#define LOW_CLUSTER_OFFSET      26
#define SIZE_OFFSET             28

#define INVALID_ENTRY           (char) (0xE5)

#define FAT_PART_TYPE           0x0C

typedef struct fat_fsinfo fat_fsinfo_t;
typedef struct fat_volume fat_volume_t;
typedef struct fat_table fat_table_t;
typedef struct fat_fs fat_fs_t;
typedef struct cache cache_t;
typedef struct cache_line cache_line_t;
typedef struct entry entry_t;
typedef struct file file_t;
typedef struct dir dir_t;

struct fat_volume
{
    struct disk *drive;
    char *label;
    uint32_t sector_count;
    uint32_t cluster_count;
    size_t sector_size;
    size_t cluster_size; // In sectors
    size_t cluster_sizeb;
};

struct fat_fsinfo
{
    uint32_t sector;
    uint8_t *buffer;
    uint32_t root_cluster;
    uint32_t data_region;
    uint32_t free_cluster_count;
    uint32_t free_cluster;
};

struct fat_table
{
    uint32_t address;
    size_t size;
    uint32_t count;
    cache_t *cache;
};

struct fat_fs
{
    fat_volume_t *volume;
    fat_table_t *table;
    fat_fsinfo_t info;
    dir_t *root_dir;
};

struct cache_line 
{
    int valid;
    int tag;
    uint8_t *data;
};

struct cache
{
    size_t cache_size;
    size_t block_size;
    uint8_t *(*read) (fat_fs_t *, uint32_t);
    uint8_t (*write) (fat_fs_t *, uint32_t, uint8_t *);
    cache_line_t *lines;
};

struct short_name 
{
    char name[FILENAME_LEN];
    char ext[FILE_EXT_LEN];
};

struct entry
{
    char short_name[SHORT_NAME_LEN];
    uint8_t attr;
    uint16_t reserved;
    uint32_t ctime;
    uint16_t atime;
    uint16_t high_cluster;
    uint32_t mtime;
    uint16_t low_cluster;
    uint32_t size;
}__packed;

struct file
{
    char *path;
    entry_t *entry;
    cache_t *cache;
    uint32_t cluster;
};

struct dir
{
    file_t *ident;
    size_t num_entries;
    entry_t **entries;
};

// src/cache.c

cache_t *cache_init(size_t cache_size, size_t block_size, uint8_t *(*read_fun)(fat_fs_t *, uint32_t), uint8_t (*write_fun) (fat_fs_t *, uint32_t, uint8_t *));
cache_line_t *cache_lines_create(size_t line_count);
uint8_t cache_readb(cache_t *cache, fat_fs_t *fs, uint32_t sector, uint32_t offset);
uint32_t cache_readl(cache_t *cache, fat_fs_t *fs, uint32_t sector, uint32_t offset);
void cache_writeb(cache_t *cache, fat_fs_t *fs, uint32_t sector, uint32_t offset, uint8_t data);
void cache_writel(cache_t *cache, fat_fs_t *fs, uint32_t sector, uint32_t offset, uint32_t data);
void cache_flush(cache_t *cache, fat_fs_t *fs);
void cache_lines_destroy(cache_line_t *cache_lines, size_t line_count);
void cache_fini(cache_t *cache);

// src/fs.c
fat_volume_t *fat_volume_init(struct disk *drive);
uint8_t *read_sector(fat_fs_t *fs, uint32_t lba);
uint8_t *read_sectors(fat_fs_t *fs, uint32_t lba, uint32_t n);
uint8_t write_sector(fat_fs_t *fs, uint32_t lba, uint8_t *buffer);
void fat_volume_fini(fat_volume_t *volume);
uint8_t *read_cluster(fat_fs_t *fs, uint32_t cluster);
uint8_t write_cluster(fat_fs_t *fs, uint32_t cluster, uint8_t *buffer);

fat_fs_t *fat_fs_init(struct disk *partition);
uint8_t fat_fs_getinfo(fat_fs_t *fs);
void fat_fs_fini(fat_fs_t *fs);
void fat_fs_printinfo(fat_fs_t *fs);

// src/table.c
fat_table_t *fat_table_init(fat_volume_t *volume);
void fat_table_fini(fat_table_t *table, fat_fs_t *fs);
uint32_t fat_table_read(fat_fs_t *fs, uint32_t cluster);
void fat_table_write(fat_fs_t *fs, uint32_t cluster, uint32_t content);
uint32_t free_cluster_count_read(fat_fs_t *fs);
uint32_t first_free_cluster_read(fat_fs_t *fs);
uint32_t fat_table_alloc_cluster(fat_fs_t *fs, uint32_t content);
uint32_t cluster_chain_get_len(fat_fs_t *fs, uint32_t start);
uint32_t cluster_chain_read(fat_fs_t *fs, uint32_t curr, uint32_t index);

// src/file.c
void rstrip_path(char *path);
file_t *file_open(fat_fs_t *fs, entry_t *entry);
void file_close(fat_fs_t *fs, file_t *file);
uint8_t file_readb(file_t *file, fat_fs_t *fs, uint32_t offset);
uint8_t *file_read(file_t *file, fat_fs_t *fs, uint32_t offset, size_t size);
void file_writeb(file_t *file, fat_fs_t *fs, uint32_t offset, uint8_t data);
void file_write(file_t *file, fat_fs_t *fs, uint32_t offset, uint8_t *data, size_t size);
uint8_t file_create(fat_fs_t *fs, char *path, char *filename);
void file_delete(fat_fs_t *fs, char *path);
file_t *file_open_path(fat_fs_t *fs, char *path);
entry_t *file_entry_create(char *filename, uint32_t cluster);

// src/dir.c
dir_t *dir_init(fat_fs_t *fs, entry_t *entry);
void dir_scan(fat_fs_t *fs, dir_t *dir);
entry_t *dir_search(dir_t *dir, char *name);
entry_t *dir_search_path(fat_fs_t *fs, dir_t *dir, char *path);
void dir_entry_create(fat_fs_t *fs, dir_t *dir, entry_t *entry);
void dir_entry_override(fat_fs_t *fs, dir_t *dir, char *short_name, entry_t *entry);
dir_t *dir_open_path(fat_fs_t *fs, char *path);
void dir_close(fat_fs_t *fs, dir_t *dir);

#endif