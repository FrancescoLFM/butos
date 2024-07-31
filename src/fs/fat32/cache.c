#include <libs/alloc.h>
#include <libs/print.h>
#include <fs/fat.h>

cache_t *cache_init(size_t cache_size, size_t block_size, uint8_t *(*read_fun)(fat_fs_t *, uint32_t), uint8_t (*write_fun) (fat_fs_t *, uint32_t, uint8_t *))
{
    cache_t *cache;

    cache = kalloc(sizeof(*cache));
    if (cache == NULL) {
        puts("Malloc error: not enough space to allocate cache");
        return NULL;
    }
    cache->cache_size = cache_size;
    cache->block_size = block_size;
    cache->lines = cache_lines_create(cache_size);
    if (cache->lines == NULL) {
        kfree(cache);
        return NULL;
    }

    cache->read = read_fun;
    cache->write = write_fun;
    
    return cache;
}

cache_line_t *cache_lines_create(size_t line_count)
{
    cache_line_t *cache_lines;

    cache_lines = kcalloc(sizeof(*cache_lines), line_count);
    if (cache_lines == NULL) {
        puts("Malloc error: not enough space to allocate cache lines vector");
        return NULL;
    }

    for (size_t i=0; i < line_count; i++) {
        cache_lines[i].valid = 0;
        cache_lines[i].tag = -1;
        cache_lines[i].data = NULL;
    }

    return cache_lines;
}

uint8_t cache_access(cache_t *cache, fat_fs_t *fs, uint32_t sector, uint32_t offset, uint8_t data, uint8_t mode)
{
    int tag;
    int index;

    tag = sector + (offset / cache->block_size);
    index = tag % cache->cache_size;

    if (cache->lines[index].tag != tag) {
        if (cache->lines[index].valid != 0)
            kfree(cache->lines[index].data);
        
        cache->lines[index].data = cache->read(fs, tag);
        if (cache->lines[index].data != NULL) {
            cache->lines[index].tag = tag;
            cache->lines[index].valid = 1;
        } 
        else
            cache->lines[index].valid = 0;
    }

    if (cache->lines[index].valid == 0)
        return 0;

    if (mode == CACHE_READ)
        return cache->lines[index].data[offset % cache->block_size];
    else if (mode == CACHE_WRITE)
        cache->lines[index].data[offset % cache->block_size] = data;

    return 0;
}

uint8_t cache_readb(cache_t *cache, fat_fs_t *fs, uint32_t sector, uint32_t offset)
{
    return cache_access(cache, fs, sector, offset, 0, CACHE_READ);
}

uint32_t cache_readl(cache_t *cache, fat_fs_t *fs, uint32_t sector, uint32_t offset)
{
    return (cache_readb(cache, fs, sector, offset)     +
    (cache_readb(cache, fs, sector, offset + 1) << 8)  +
    (cache_readb(cache, fs, sector, offset + 2) << 16) +
    (cache_readb(cache, fs, sector, offset + 3) << 24));
}

void cache_writeb(cache_t *cache, fat_fs_t *fs, uint32_t sector, uint32_t offset, uint8_t data)
{
    cache_access(cache, fs, sector, offset, data, CACHE_WRITE);
}

void cache_writel(cache_t *cache, fat_fs_t *fs, uint32_t sector, uint32_t offset, uint32_t data)
{
    cache_writeb(cache, fs, sector, offset, data);
    cache_writeb(cache, fs, sector, offset + 1, data >> 8);
    cache_writeb(cache, fs, sector, offset + 2, data >> 16);
    cache_writeb(cache, fs, sector, offset + 3, data >> 24);
}

void cache_flush(cache_t *cache, fat_fs_t *fs)
{
    for (size_t i=0; i < cache->cache_size; i++)
        if (cache->lines[i].valid && cache->lines[i].tag != -1) {
            if (cache->write(fs, cache->lines[i].tag, cache->lines[i].data))
                printk("Failed to flush cache line: %d\n", i);
        }
}

void cache_lines_destroy(cache_line_t *cache_lines, size_t line_count)
{
    for (size_t i=0; i < line_count; i++)
        if (cache_lines[i].data != NULL)
            kfree(cache_lines[i].data); //! KPANIC

    kfree(cache_lines);
}

void cache_fini(cache_t *cache)
{
    cache_lines_destroy(cache->lines, cache->cache_size);
    kfree(cache);
}