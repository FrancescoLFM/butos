#include <fs/fd.h>
#include <cpu/proc.h>
#include <libs/alloc.h>
#include <libs/string.h>

struct vfd_entry {
    int used;
    char path[VFD_PATH_MAX];
    uint8_t *buf;
    size_t size;
    int writable;
};

struct fd_entry {
    int used;
    int dirty;
    /* Process nesting depth that opened the file */
    int owner;
    uint32_t pos;
    fd_type_t type;
    union {
        file_t *file;
        struct {
            uint8_t *buf;
            size_t   size;
            int      writable;
        } ram;
    };
};

static fat_fs_t *fd_root_fs;
static struct fd_entry fd_table[FD_MAX];
static struct vfd_entry vfd_table[VFD_MAX];

void fd_init(fat_fs_t *root_fs)
{
    fd_root_fs = root_fs;
    memset(fd_table, 0, sizeof(fd_table));
}

static struct fd_entry *fd_get(int fd)
{
    if (fd < 0 || fd >= FD_MAX || !fd_table[fd].used)
        return NULL;

    return &fd_table[fd];
}

fd_status_t vfd_register(const char *path, void *buf, size_t size, int writable)
{
    int i;

    if (path == NULL || *path != '/' || buf == NULL)
        return FD_INVALID;
    if (strlen(path) >= VFD_PATH_MAX)
        return FD_INVALID;

    for (i = 0; i < VFD_MAX; i++)
        if (vfd_table[i].used && strcmp(vfd_table[i].path, path) == 0)
            return FD_EXISTS;

    for (i = 0; i < VFD_MAX; i++) {
        if (!vfd_table[i].used) {
            strncpy(vfd_table[i].path, path, VFD_PATH_MAX - 1);
            vfd_table[i].path[VFD_PATH_MAX - 1] = '\0';
            vfd_table[i].buf      = (uint8_t *) buf;
            vfd_table[i].size     = size;
            vfd_table[i].writable = writable;
            vfd_table[i].used     = 1;
            return FD_OK;
        }
    }
    return FD_TOO_MANY;
}

fd_status_t vfd_unregister(const char *path)
{
    int i, fd;

    if (path == NULL)
        return FD_INVALID;

    for (i = 0; i < VFD_MAX; i++) {
        if (!vfd_table[i].used || strcmp(vfd_table[i].path, path) != 0)
            continue;
        for (fd = 0; fd < FD_MAX; fd++) {
            if (fd_table[fd].used && fd_table[fd].type == FD_TYPE_RAM &&
                fd_table[fd].ram.buf == vfd_table[i].buf)
                return FD_BUSY;
        }
        vfd_table[i].used = 0;
        return FD_OK;
    }
    return FD_NOT_FOUND;
}

int fd_open(char *path)
{
    int fd, i;

    if (path == NULL || *path != '/')
        return FD_INVALID;

    for (fd = 0; fd < FD_MAX && fd_table[fd].used; fd++);
    if (fd == FD_MAX)
        return FD_TOO_MANY;

    for (i = 0; i < VFD_MAX; i++) {
        if (!vfd_table[i].used || strcmp(vfd_table[i].path, path) != 0)
            continue;
        fd_table[fd].type         = FD_TYPE_RAM;
        fd_table[fd].ram.buf      = vfd_table[i].buf;
        fd_table[fd].ram.size     = vfd_table[i].size;
        fd_table[fd].ram.writable = vfd_table[i].writable;
        fd_table[fd].used         = 1;
        fd_table[fd].dirty        = 0;
        fd_table[fd].owner        = process_depth();
        fd_table[fd].pos          = 0;
        return fd;
    }

    if (fd_root_fs == NULL)
        return FD_INVALID;

    fd_table[fd].file = file_open_path(fd_root_fs, path);
    if (fd_table[fd].file == NULL)
        return FD_NOT_FOUND;
    fd_table[fd].type  = FD_TYPE_FAT32;
    fd_table[fd].used  = 1;
    fd_table[fd].dirty = 0;
    fd_table[fd].owner = process_depth();
    fd_table[fd].pos   = 0;

    return fd;
}

fd_status_t fd_close(int fd)
{
    struct fd_entry *entry;
    fd_status_t status = FD_OK;

    entry = fd_get(fd);
    if (entry == NULL)
        return FD_INVALID;

    if (entry->type == FD_TYPE_FAT32) {
        if (entry->dirty && file_sync(fd_root_fs, entry->file))
            status = FD_IO_ERROR;
        file_close(fd_root_fs, entry->file);
        entry->file = NULL;
    }
    entry->used = 0;

    return status;
}

int fd_read(int fd, void *buffer, size_t size)
{
    struct fd_entry *entry;
    uint8_t *out = buffer;
    size_t avail, to_read, i;

    entry = fd_get(fd);
    if (entry == NULL || buffer == NULL)
        return FD_INVALID;

    if (entry->type == FD_TYPE_RAM) {
        avail   = (entry->pos < entry->ram.size) ? entry->ram.size - entry->pos : 0;
        to_read = (size < avail) ? size : avail;
        memcpy(out, entry->ram.buf + entry->pos, to_read);
        entry->pos += (uint32_t) to_read;
        return (int) to_read;
    }

    for (i = 0; i < size && entry->pos < entry->file->entry->size; i++, entry->pos++)
        out[i] = file_readb(entry->file, fd_root_fs, entry->pos);

    return (int) i;
}

int fd_write(int fd, void *buffer, size_t size)
{
    struct fd_entry *entry;
    size_t written;

    entry = fd_get(fd);
    if (entry == NULL || buffer == NULL)
        return FD_INVALID;
    if (size == 0)
        return 0;

    if (entry->type == FD_TYPE_RAM) {
        if (!entry->ram.writable)
            return FD_IO_ERROR;
        if (entry->pos >= entry->ram.size)
            return FD_IO_ERROR;
        written = entry->ram.size - entry->pos;
        if (size < written)
            written = size;
        memcpy(entry->ram.buf + entry->pos, (uint8_t *) buffer, written);
        entry->pos += (uint32_t) written;
        return (int) written;
    }

    written = file_write(entry->file, fd_root_fs, entry->pos, buffer, size);
    entry->pos += written;
    if (written)
        entry->dirty = 1;
    if (written == 0)
        return FD_IO_ERROR;

    return (int) written;
}

int fd_seek(int fd, int offset, int whence)
{
    struct fd_entry *entry;
    uint32_t file_size;
    int base;
    int pos;

    entry = fd_get(fd);
    if (entry == NULL)
        return FD_INVALID;

    file_size = (entry->type == FD_TYPE_FAT32)
        ? entry->file->entry->size
        : (uint32_t) entry->ram.size;

    switch (whence) {
    case FD_SEEK_SET:
        base = 0;
        break;
    case FD_SEEK_CUR:
        base = (int) entry->pos;
        break;
    case FD_SEEK_END:
        base = (int) file_size;
        break;
    default:
        return FD_INVALID;
    }

    pos = base + offset;
    if (pos < 0 || (uint32_t) pos > file_size)
        return FD_INVALID;
    entry->pos = (uint32_t) pos;

    return pos;
}

static int short_name_char_valid(char c)
{
    const char *invalid = "\"*+,./:;<=>?[\\]| ";

    if (c <= 0x20 || c == 0x7F)
        return 0;
    for (; *invalid; invalid++)
        if (c == *invalid)
            return 0;

    return 1;
}

/* Only 8.3 names are supported, without long file names */
static int short_name_valid(char *name)
{
    size_t i, j;

    for (i = 0; name[i] && name[i] != '.'; i++)
        if (i >= FILENAME_LEN || !short_name_char_valid(name[i]))
            return 0;
    if (i == 0)
        return 0;
    if (name[i] == '\0')
        return 1;

    for (j = 0, i++; name[i]; i++, j++)
        if (j >= FILE_EXT_LEN || !short_name_char_valid(name[i]))
            return 0;

    return j > 0;
}

fd_status_t fd_create(char *path)
{
    char *dir_path;
    char *name;
    file_t *file;
    fd_status_t status = FD_OK;
    int i;

    if (path == NULL || *path != '/')
        return FD_INVALID;

    for (i = 0; i < VFD_MAX; i++)
        if (vfd_table[i].used && strcmp(vfd_table[i].path, path) == 0)
            return FD_EXISTS;

    if (fd_root_fs == NULL)
        return FD_INVALID;

    file = file_open_path(fd_root_fs, path);
    if (file != NULL) {
        file_close(fd_root_fs, file);
        return FD_EXISTS;
    }

    /* Split "/dir/name" into "/dir" and "name" */
    dir_path = strdup(path);
    if (dir_path == NULL)
        return FD_IO_ERROR;
    name = dir_path + strlen(dir_path);
    while (*name != '/')
        name--;
    *name++ = '\0';

    if (!short_name_valid(name))
        status = FD_INVALID;
    else if (file_create(fd_root_fs, *dir_path ? dir_path : "/", name))
        status = FD_IO_ERROR;

    kfree(dir_path);

    return status;
}

fd_status_t fd_remove(char *path)
{
    file_t *file;
    file_t *open;
    int i;

    if (path == NULL || *path != '/')
        return FD_INVALID;

    for (i = 0; i < VFD_MAX; i++)
        if (vfd_table[i].used && strcmp(vfd_table[i].path, path) == 0)
            return FD_INVALID;

    if (fd_root_fs == NULL)
        return FD_INVALID;

    file = file_open_path(fd_root_fs, path);
    if (file == NULL)
        return FD_NOT_FOUND;

    /* Removing an open file would leave its descriptor pointing to freed clusters */
    for (int fd = 0; fd < FD_MAX; fd++) {
        if (!fd_table[fd].used || fd_table[fd].type != FD_TYPE_FAT32)
            continue;
        open = fd_table[fd].file;
        if (open->path != NULL && file->path != NULL && !strcmp(open->path, file->path) &&
            !strncmp(open->entry->short_name, file->entry->short_name, SHORT_NAME_LEN)) {
            file_close(fd_root_fs, file);
            return FD_BUSY;
        }
    }
    file_close(fd_root_fs, file);

    if (file_delete(fd_root_fs, path))
        return FD_IO_ERROR;

    return FD_OK;
}

void fd_close_owned(int owner)
{
    for (int fd = 0; fd < FD_MAX; fd++)
        if (fd_table[fd].used && fd_table[fd].owner >= owner)
            fd_close(fd);
}

static void decode_83(const char *raw, char *out)
{
    int i, j = 0;
    char c;

    for (i = 0; i < FILENAME_LEN && raw[i] != ' '; i++) {
        c = raw[i];
        out[j++] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
    }
    if (raw[FILENAME_LEN] != ' ') {
        out[j++] = '.';
        for (i = FILENAME_LEN; i < SHORT_NAME_LEN && raw[i] != ' '; i++) {
            c = raw[i];
            out[j++] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
        }
    }
    out[j] = '\0';
}

int fd_readdir(char *path, int index, fd_dirent_t *out)
{
    dir_t *dir;
    int real_idx = 0;
    size_t i;
    entry_t *e;

    if (fd_root_fs == NULL || path == NULL || out == NULL || index < 0)
        return FD_INVALID;

    dir = dir_open_path(fd_root_fs, path);
    if (dir == NULL)
        return FD_NOT_FOUND;

    dir_scan(fd_root_fs, dir);

    for (i = 0; i < dir->num_entries; i++) {
        e = dir->entries[i];
        if (e == NULL)
            break;
        if (e->short_name[0] == '.')
            continue;
        if (real_idx == index) {
            decode_83(e->short_name, out->name);
            out->is_dir = (e->attr & DIR_ATTR) != 0;
            out->size   = e->size;
            dir_close(fd_root_fs, dir);
            return 1;
        }
        real_idx++;
    }

    dir_close(fd_root_fs, dir);
    return 0;
}
