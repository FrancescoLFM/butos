#include <include/asm.h>
#include <libs/print.h>

#include <cpu/idt.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <libs/alloc.h>
#include <cpu/proc.h>
#include <cpu/paging.h>
#include <libs/string.h>
#include <fs/disk.h>
#include <fs/fat.h>

void test_allocator();
void elf_test();

extern char __bss_start[], __bss_end[];

#define INIT_PATH   "/butosh"

/* Mounts the FAT32 partition and runs the init process (the shell), respawning it on exit */
static void init_start()
{
    struct disk *disk;
    fat_fs_t *fs;
    proc_status_t status;
    int exit_code;

    disk = disk_init(ATA_DRIVE);
    if (disk == NULL) {
        puts("Failed to open the boot disk\n");
        return;
    }
    if (disk_set_offset(disk, FAT_PART_TYPE)) {
        puts("No FAT32 partition on the boot disk\n");
        disk_fini(disk);
        return;
    }
    fs = fat_fs_init(disk);
    if (fs == NULL) {
        puts("Failed to mount the FAT32 partition\n");
        disk_fini(disk);
        return;
    }
    if (process_init(fs)) {
        puts("Failed to initialize processes\n");
        fat_fs_fini(fs);
        disk_fini(disk);
        return;
    }

    for (;;) {
        status = process_spawn(INIT_PATH, &exit_code);
        if (status != PROC_OK) {
            printk("Failed to start %s (error %d)\n", INIT_PATH, status);
            return;
        }
        printk("%s exited with code %d, restarting\n", INIT_PATH, exit_code);
    }
}

/* Placed at 0xC0000000 by butos.ld, stage 2 jumps there */
__attribute__((section(".text.entry"))) void main()
{
    /* The bootloader doesn't clear .bss, zero-initialized globals rely on this */
    memset(__bss_start, 0, __bss_end - __bss_start);
    page_directory_adjust();
    isr_install();
    vga_open();
    vga_clear(BLACK);

    void *heap_start = (void *) 0xC00f0000;
    size_t heap_size = 492032;
    void *registry_start = (unsigned char *)heap_start + heap_size;
    size_t registry_capacity = 100 * 8;

    kalloc_start(
        heap_start,
        heap_size,
        registry_start,
        registry_capacity
    );

    keyboard_start(100);

    init_start();

    stop();
}
