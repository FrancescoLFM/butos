#include <fs/fat.h>
#include <libs/alloc.h>
#include <libs/string.h>
#include <libs/print.h>

#define TOLOWER(C)   ((C >= 'A' && C <= 'Z') ? C + 32 : C) 

void entry_array_destroy(entry_t **entries, size_t num_entries)
{
    for (size_t i = 0; i < num_entries; i++) {
        if (entries[i] != NULL)
            kfree(entries[i]);
    }

    kfree(entries);
}

entry_t **entry_array_create(size_t num_entries)
{
    entry_t **entries;

    entries = kcalloc(num_entries, sizeof(*entries));
    if (entries == NULL)
        return NULL;
    
    for (size_t i = 0; i < num_entries; i++) {
        entries[i] = kcalloc(1, sizeof(*entries[i]));
        if (entries[i] == NULL) {
            entry_array_destroy(entries, num_entries);
            return NULL;
        }
    }

    return entries;
}

dir_t *dir_init(fat_fs_t *fs, entry_t *entry)
{
    dir_t *dir;
    size_t raw_size;
    
    raw_size = cluster_chain_get_len(fs, WORDS_TO_LONG(entry->high_cluster, entry->low_cluster)) * fs->volume->cluster_sizeb;

    dir = kalloc(sizeof(*dir));
    if (dir == NULL) {
        puts("Malloc error: not enough space to allocate directory");
        return NULL;
    }

    // Alloco il vettore di entry della directory
    dir->num_entries = raw_size / sizeof(*entry);
    dir->entries = entry_array_create(dir->num_entries);
    if (!dir->entries) {
        kfree(dir);
        return NULL;
    }

    // Apro la directory come file
    entry->size = raw_size;
    dir->ident = file_open(fs, entry);
    if (dir->ident == NULL) {
        entry_array_destroy(dir->entries, dir->num_entries);
        kfree(dir);
        return NULL;
    }

    return dir;
}

void dir_entry_create(fat_fs_t *fs, dir_t *dir, entry_t *entry)
{
    size_t offset = 0;
    uint8_t entry_tag;

    while (offset < dir->ident->entry->size) {
        entry_tag = file_readb(dir->ident, fs, offset);
        if (entry_tag == 0 || entry_tag == (uint8_t) INVALID_ENTRY) {
            file_write(dir->ident, fs, offset, (uint8_t *) entry, sizeof(*entry));
            return;
        }   
        offset += sizeof(entry_t);
    }
}

void dir_entry_override(fat_fs_t *fs, dir_t *dir, char *short_name, entry_t *entry)
{
    size_t offset = 0;
    entry_t *curr_entry;

    while (offset < dir->ident->entry->size) {
        curr_entry = (entry_t *) file_read(dir->ident, fs, offset, sizeof(*curr_entry));
        if (curr_entry == NULL)
            return;

        if (!strncmp(curr_entry->short_name, short_name, SHORT_NAME_LEN)) {
            file_write(dir->ident, fs, offset, (uint8_t *) entry, sizeof(*entry));
            kfree(curr_entry);
            return;
        }

        offset += sizeof(entry_t);
        kfree(curr_entry);
    }

    return;
}

entry_t *dir_read_entry(fat_fs_t *fs, dir_t *dir, size_t offset)
{
    uint8_t *raw_entry;
    entry_t *ret;

    ret = kcalloc(1, sizeof(*ret));
    raw_entry = file_read(dir->ident, fs, offset, sizeof(*ret));
    strncpy(ret->short_name, (char *) raw_entry, SHORT_NAME_LEN);
    ret->attr = raw_entry[ATTR_OFFSET];
    ret->low_cluster = BYTES_TO_WORD(raw_entry, LOW_CLUSTER_OFFSET);
    ret->ctime = BYTES_TO_LONG(raw_entry, CTIME_OFFSET);
    ret->atime = BYTES_TO_LONG(raw_entry, ATIME_OFFSET);
    ret->high_cluster = BYTES_TO_LONG(raw_entry, HIGH_CLUSTER_OFFSET);
    ret->mtime = BYTES_TO_LONG(raw_entry, MTIME_OFFSET);
    ret->size = BYTES_TO_LONG(raw_entry, SIZE_OFFSET);

    kfree(raw_entry);
    return ret;
}

void dir_scan(fat_fs_t *fs, dir_t *dir)
{
    size_t offset = 0;
    uint8_t entry_attr;
    entry_t *temp_entry;

    for (size_t i=0; offset < dir->ident->entry->size;) {
        entry_attr = file_readb(dir->ident, fs, offset + ATTR_OFFSET);
        if (entry_attr == LFN_ATTR)
            offset += sizeof(entry_t);
        if (entry_attr != 0) {
            temp_entry = dir_read_entry(fs, dir, offset);
            if (temp_entry->short_name[0] != INVALID_ENTRY && temp_entry->short_name[0] != '\0') {
                if (dir->entries[i] != NULL)
                    kfree(dir->entries[i]);
                dir->entries[i++] = temp_entry;
            }
            else 
                kfree(temp_entry);
        }
        offset += sizeof(entry_t);
    }
}

int strcmp_insensitive(char *a, char *b)
{
    size_t i = 0;

    do {
        if (!a[i] || !b[i])
            return a[i]-b[i];
        i++;
    } while (TOLOWER(a[i]) == TOLOWER(b[i]));

    return a[i] - b[i];
}

int compare_short_name(char* name, char* str) {
    int i;
    char short_name_str[12];

    struct short_name *short_name = (struct short_name *) name;

    for (i = 0; i < 8 && short_name->name[i] != ' '; i++) {
        short_name_str[i] = short_name->name[i];
    }

    // Se c'è un'estensione, aggiungila con un '.'
    if (short_name->ext[0] != ' ') {
        short_name_str[i++] = '.';
        int j;
        for (j = 0; j < 3 && short_name->ext[j] != ' '; j++, i++) {
            short_name_str[i] = short_name->ext[j];
        }
    }

    // Aggiungi il terminatore di stringa
    short_name_str[i] = '\0';

    // Confronta la stringa temporanea con la stringa passata come argomento
    return strcmp_insensitive(short_name_str, str);
}

entry_t *dir_search(dir_t *dir, char *name)
{
    size_t i = 0;
    entry_t *ret;

    do {
        if(dir->entries[i] != NULL &&
            !compare_short_name(dir->entries[i]->short_name, name)) {
            ret = kalloc(sizeof(*ret));
            if (ret == NULL)
                return NULL;
            memcpy(ret, dir->entries[i], sizeof(*ret));
            return ret;
        }
    } while(++i < dir->num_entries);

    return NULL;
}

entry_t *dir_search_path(fat_fs_t *fs, dir_t *dir, char *path)
{
    char filename[12];
    dir_t *curr_dir = dir;
    size_t i = 0;
    entry_t *entry = NULL;
    entry_t *ret = NULL;

    // Get filename
    while (*path != '/' && i < 11 && *path) {
        filename[i++] = *path++;
    }
    filename[i] = '\0';

    entry = dir_search(curr_dir, filename);
    if (entry == NULL)
        return NULL;
    
    if (*path == '/') {
        curr_dir = dir_init(fs, entry);
        if (curr_dir == NULL) {
            kfree(entry);
            return NULL;
        }
        // Compie la ricerca
        dir_scan(fs, curr_dir);
        ret = dir_search_path(fs, curr_dir, ++path);
    
        dir_close(fs, curr_dir);
        kfree(entry);
        if (ret == NULL)
            return NULL;
    } else if (*path == '\0') { /* caso base */
        return entry;
    } else {
        kfree(entry);
    }

    return ret;
}

dir_t *dir_open_path(fat_fs_t *fs, char *path)
{
    dir_t *starting_dir;
    entry_t *entry_dir;
    dir_t *dir;

    if (*path == '/') {
        starting_dir = fs->root_dir;
        if(*++path == '\0') {
            dir = dir_init(fs, fs->root_dir->ident->entry);
            return dir;
        }
    }
    // Non supportiamo la path relativa
    else
        return NULL;

    dir_scan(fs, starting_dir);
    entry_dir = dir_search_path(fs, starting_dir, path);
    if (entry_dir->attr != DIR_ATTR) {
        // kfree(entry_dir);
        return NULL;
    }

    dir = dir_init(fs, entry_dir);
    if (dir == NULL) {
        kfree(entry_dir);
        return NULL;
    }

    return dir;
}

void dir_close(fat_fs_t *fs, dir_t *dir)
{
    entry_array_destroy(dir->entries, dir->num_entries);
    file_close(fs, dir->ident);
    kfree(dir);
}