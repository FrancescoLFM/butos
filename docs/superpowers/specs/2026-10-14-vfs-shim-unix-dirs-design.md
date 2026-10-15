# VFS Shim + Unix Directory Layout

## Goal

1. File virtuali (buffer RAM) accessibili via path normali attraverso le file syscalls esistenti.
2. Albero directory Unix di default sul disco FAT32 (`/usr`, `/usr/bin`, `/bin`, `/etc`, `/tmp`).
3. Programmi copiati in `/usr/bin`; shell cerca i comandi in `/usr/bin`, `/bin`, poi `/`.

---

## Architettura

### Strato VFD (Virtual File Descriptor)

`fd.c` guadagna una tabella statica di file virtuali `vfd_table[]` separata dalla tabella dei fd aperti. Ogni entry tiene `path`, `buf`, `size`, `writable`.

`fd_open` controlla prima `vfd_table` per match esatto del path; se trovato, alloca un `fd_entry` di tipo RAM invece di aprire un file FAT32.

`fd_entry` guadagna un campo `type` (`FD_TYPE_FAT32` | `FD_TYPE_RAM`) e una union:

```c
typedef enum { FD_TYPE_FAT32, FD_TYPE_RAM } fd_type_t;

struct fd_entry {
    int used;
    int dirty;   /* solo FAT32 */
    int owner;
    uint32_t pos;
    fd_type_t type;
    union {
        file_t *file;          /* FD_TYPE_FAT32 */
        struct {
            uint8_t *buf;
            size_t   size;
            int      writable;
        } ram;                 /* FD_TYPE_RAM */
    };
};
```

### API nuova (fd.h)

```c
#define VFD_MAX       8
#define VFD_PATH_MAX  64

/* Registra un file virtuale (chiamabile prima o dopo fd_init) */
fd_status_t vfd_register(const char *path, void *buf, size_t size, int writable);
/* Deregistra; fallisce se il file è aperto */
fd_status_t vfd_unregister(const char *path);
```

### Dispatch in fd_read / fd_write / fd_seek / fd_close

- **FD_TYPE_FAT32**: comportamento invariato.
- **FD_TYPE_RAM read**: `memcpy` da `buf+pos`, clamped a `size`.
- **FD_TYPE_RAM write** (se `writable`): `memcpy` in `buf+pos`, clamped a `size` — buffer fisso, no grow. Ritorna `FD_IO_ERROR` se non writable.
- **FD_TYPE_RAM seek**: `SEEK_END` usa `ram.size`.
- **FD_TYPE_RAM close**: marca entry unused, nessun sync.
- **fd_create / fd_remove**: solo FAT32; path virtuali ritornano `FD_INVALID`.

### Directory Unix — build time

Nel top-level `makefile`, dopo `mkfs.fat`, aggiungere prima di `mcopy`:

```makefile
mmd -i $(PART) ::/usr ::/usr/bin ::/bin ::/etc ::/tmp
```

Cambiare la riga `mcopy` da `::/` a `::/usr/bin/`:

```makefile
mcopy -o -i $(PART) $(PROGBIN) ::/usr/bin/
```

No kernel-side mkdir richiesto.

### Init path

`src/main.c`:
```c
#define INIT_PATH   "/usr/bin/butosh"
```

### Butosh PATH search

`program_execute` in `butosh.c` prova in ordine:
1. Se il nome contiene `/` — path assoluto, usalo direttamente.
2. `/usr/bin/<name>`
3. `/bin/<name>`
4. `/<name>` (compatibilità retroattiva)

Primo `exec_syscall` che non ritorna `EXEC_NOT_FOUND` vince.

---

## File toccati

| File | Cambiamento |
|------|-------------|
| `src/fs/fd.h` | `fd_type_t`, nuovo `fd_entry`, `vfd_register`, `vfd_unregister`, `VFD_MAX`, `VFD_PATH_MAX` |
| `src/fs/fd.c` | tabella `vfd_table`, impl `vfd_register/unregister`, dispatch per tipo in tutte le operazioni |
| `src/main.c` | `INIT_PATH = "/usr/bin/butosh"` |
| `makefile` | `mmd` per dirs, `mcopy` target a `::/usr/bin/` |
| `programs/sources/butosh/butosh.c` | PATH search in `program_execute` |

---

## Invarianti e limiti

- Buffer RAM fisso: `vfd_register` non copia il buffer, tiene il puntatore — il chiamante deve garantire lifetime.
- File virtuali non appaiono in `ls` (nessun readdir virtuale — futura estensione).
- `FD_MAX 16` non cambia; file virtuali aperti consumano slot come quelli FAT32.
- Nomi 8.3 non richiesti per path virtuali (no FAT32 constraint).
- `fd_close_owned` chiude anche fd RAM (già coperto dal loop esistente su `fd_table`).
