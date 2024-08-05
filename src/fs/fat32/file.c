#include <fs/fat.h>
#include <libs/alloc.h>
#include <libs/string.h>
#include <libs/print.h>

file_t *file_open(fat_fs_t *fs, entry_t *entry)
{
    file_t *file;

    file = kalloc(sizeof(*file));
    if (file == NULL) {
        puts("Malloc error: not enough space to allocate file");
        return NULL;
    }

    file->path = NULL;
    file->entry = kalloc(sizeof(*file->entry));
    if (file->entry == NULL) {
        kfree(file);
        puts("Malloc error: not enough space to allocate file entry");
        return NULL;
    }
    memcpy(file->entry, entry, sizeof(*file->entry));
    file->cache = cache_init((entry->size / fs->volume->cluster_sizeb) + 1, fs->volume->cluster_sizeb, 
                              read_cluster, write_cluster);
    if (file->cache == NULL) {
        puts("File error: not enough space to init the file cache");
        kfree(file->entry);
        kfree(file);
        return NULL;
    }
    file->cluster = WORDS_TO_LONG(entry->high_cluster, entry->low_cluster);

    return file;
}

uint8_t file_readb(file_t *file, fat_fs_t *fs, uint32_t offset)
{
    uint32_t cluster;

    if (offset > file->entry->size)
        return FAT_EOF;

    cluster = cluster_chain_read(fs, file->cluster, offset / fs->volume->cluster_sizeb);
    return cache_readb(file->cache, fs, cluster, offset);
}

uint8_t *file_read(file_t *file, fat_fs_t *fs, uint32_t offset, size_t size)
{
    uint8_t *buffer;

    buffer = kalloc(size * sizeof(*buffer));
    if (buffer == NULL) {
        puts("Memory error: not enough space to save file buffer");
        return NULL;
    }

    for (size_t i=0; i < size; i++)
        buffer[i] = file_readb(file, fs, offset + i);

    return buffer;
}

void file_writeb(file_t *file, fat_fs_t *fs, uint32_t offset, uint8_t data)
{
    uint32_t cluster;
    uint32_t cluster_chain_len;
    uint32_t new_cluster;
    uint32_t curr_cluster;

    if (offset > file->entry->size)
        file->entry->size = offset;

    cluster_chain_len = cluster_chain_get_len(fs, file->cluster);
    if (((file->entry->size - 1) / fs->volume->cluster_sizeb) + 1 > cluster_chain_len) {
        curr_cluster = file->cluster;
        for (uint32_t i=0; i < (file->entry->size / fs->volume->cluster_sizeb); i++) {
            new_cluster = fat_table_alloc_cluster(fs, EOC1);
            fat_table_write(fs, curr_cluster, new_cluster);
            curr_cluster = new_cluster;
        }
    }

    cluster = cluster_chain_read(fs, file->cluster, offset / fs->volume->cluster_sizeb);
    return cache_writeb(file->cache, fs, cluster, offset, data);
}

void file_write(file_t *file, fat_fs_t *fs, uint32_t offset, uint8_t *data, size_t size)
{
    for (size_t i=0; i < size; i++)
        file_writeb(file, fs, offset + i, data[i]);
}

file_t *file_open_path(fat_fs_t *fs, char *path)
{
    entry_t *entry;
    dir_t *starting_dir;
    char *save_path = path;
    file_t *ret;

    /* root check */
    if (*path == '/') {
        starting_dir = fs->root_dir;
        ++path;
    }
    else
        return NULL;

    dir_scan(fs, starting_dir);
    entry = dir_search_path(fs, starting_dir, path);
    if (entry == NULL) 
        return NULL;
    if (entry->attr != FILE_ATTR) {
        kfree(entry);
        return NULL;
    }

    ret = file_open(fs, entry);
    ret->path = strdup(save_path);
    if (ret->path != NULL)
        rstrip_path(ret->path);

    kfree(entry);
    return ret;
}

int ctoupper(char c)
{
    if (c >= 'a' && c <= 'z')
        return c - ('a' - 'A');
    else
        return c;
}

void entry_name_copy(entry_t *entry, char *filename)
{
    int j;

    for (int i = 0; i < SHORT_NAME_LEN; i++)
        entry->short_name[i] = ' ';

    for (j = 0; j < FILENAME_LEN && filename[j]; j++) {
        if (filename[j] == '.') {
            j++;
            break;
        }
        entry->short_name[j] = ctoupper(filename[j]);
    }

    for (int k = 0; k < FILE_EXT_LEN && filename[j]; k++, j++)
        entry->short_name[FILENAME_LEN + k] = ctoupper(filename[j]);

    return;
}

entry_t *file_entry_create(char *filename, uint32_t cluster)
{
    entry_t *entry;

    entry = kcalloc(1, sizeof(*entry));
    if (entry == NULL)
        return NULL;

    entry_name_copy(entry, filename);

    entry->low_cluster = cluster & 0xFFFF;
    entry->high_cluster = cluster >> 16;
    entry->attr = FILE_ATTR;

    return entry;
}

void rstrip_path(char *path)
{
    size_t len;
    size_t i;
    len = strlen(path);

    for (i=len; i; i--)
        if (path[i] == '/') {
            path[i] = '\0';
            return;
        }

    return;
}

uint8_t file_create(fat_fs_t *fs, char *path, char *filename)
{
    uint32_t cluster;
    dir_t *dir;
    entry_t *file_entry;

    cluster = first_free_cluster_read(fs);

    dir = dir_open_path(fs, path);
    if (dir == NULL)
        return EXIT_FAILURE;
    dir_scan(fs, dir);
    if ((file_entry = dir_search(dir, filename)) != NULL) {
        kfree(file_entry);
        dir_close(fs, dir);
        return EXIT_FAILURE;
    }
    file_entry = file_entry_create(filename, cluster);
    if (file_entry == NULL) {
        dir_close(fs, dir);
        return EXIT_FAILURE;
    }
    
    fat_table_alloc_cluster(fs, EOC1);
    dir_entry_create(fs, dir, file_entry);

    dir_close(fs, dir);
    kfree(file_entry);

    return EXIT_SUCCESS;
}

void file_delete(fat_fs_t *fs, char *path)
{
    dir_t *dir;
    file_t *file;
    entry_t *dummy_entry;

    file = file_open_path(fs, path);
    if (file == NULL)
        return;
    if (file->path == NULL) {
        file_close(fs, file);
        return;
    }
    dir = dir_open_path(fs, file->path);
    if (dir == NULL) {
        file_close(fs, file);
        return;
    }
    dummy_entry = kcalloc(1, sizeof(*dummy_entry));
    if (dummy_entry == NULL) {
        dir_close(fs, dir);
        file_close(fs, file);
        return;
    }
    dir_scan(fs, dir);
    dir_entry_override(fs, dir, file->entry->short_name, dummy_entry);
    fat_table_write(fs, file->cluster, 0);

    file_close(fs, file);
    dir_close(fs, dir);
    kfree(dummy_entry);
    
    return;
}

void file_close(fat_fs_t *fs, file_t *file) 
{
    if (file->path != NULL)
        kfree(file->path);
    /* BUTOS FAILS HERE FOR SOME REASON */
    cache_flush(file->cache, fs);
    cache_fini(file->cache);
    kfree(file->entry);
    kfree(file);
}