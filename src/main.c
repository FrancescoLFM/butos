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
#include <fs/fd.h>

void test_allocator();
void elf_test();

extern char __bss_start[], __bss_end[];

#define INIT_PATH   "/butosh"

static void vfd_test(void)
{
    static char hello_buf[] = "Hello, VFD!";
    static char rw_buf[32];
    char read_buf[32];
    int fd, n;

    printk("=== vfd_test start ===\n");

    if (vfd_register("/dev/hello", hello_buf, 11, 0) != FD_OK) {
        printk("FAIL: vfd_register ro\n"); return;
    }
    fd = fd_open("/dev/hello");
    if (fd < 0) { printk("FAIL: open ro\n"); return; }
    n = fd_read(fd, read_buf, sizeof(read_buf));
    if (n != 11 || strncmp(read_buf, "Hello, VFD!", 11) != 0) {
        printk("FAIL: read content\n"); return;
    }
    if (fd_read(fd, read_buf, 1) != 0) { printk("FAIL: no EOF\n"); return; }
    if (fd_seek(fd, 0, FD_SEEK_SET) != 0) { printk("FAIL: seek set\n"); return; }
    if (fd_read(fd, read_buf, 1) != 1 || read_buf[0] != 'H') {
        printk("FAIL: seek+read\n"); return;
    }
    if (fd_seek(fd, 12, FD_SEEK_SET) != FD_INVALID) { printk("FAIL: seek past end\n"); return; }
    if (fd_write(fd, "x", 1) != FD_IO_ERROR) { printk("FAIL: write on ro\n"); return; }
    fd_close(fd);
    printk("PASS: read-only vfd\n");

    if (vfd_register("/dev/rw", rw_buf, sizeof(rw_buf), 1) != FD_OK) {
        printk("FAIL: vfd_register rw\n"); return;
    }
    fd = fd_open("/dev/rw");
    if (fd < 0) { printk("FAIL: open rw\n"); return; }
    n = fd_write(fd, "test_data", 9);
    if (n != 9) { printk("FAIL: write count\n"); return; }
    fd_seek(fd, 0, FD_SEEK_SET);
    memset(read_buf, 0, sizeof(read_buf));
    n = fd_read(fd, read_buf, 9);
    if (n != 9 || strncmp(read_buf, "test_data", 9) != 0) {
        printk("FAIL: rw readback\n"); return;
    }
    fd_seek(fd, 0, FD_SEEK_END);
    if (fd_write(fd, "x", 1) != FD_IO_ERROR) { printk("FAIL: write past end\n"); return; }
    if (vfd_unregister("/dev/rw") != FD_BUSY) { printk("FAIL: busy check\n"); return; }
    fd_close(fd);
    printk("PASS: writable vfd\n");

    if (vfd_unregister("/dev/hello") != FD_OK) { printk("FAIL: unregister\n"); return; }
    if (fd_open("/dev/hello") >= 0) { printk("FAIL: open after unregister\n"); return; }
    printk("PASS: unregister\n");

    if (vfd_register("/dev/rw", rw_buf, sizeof(rw_buf), 1) != FD_EXISTS) {
        printk("FAIL: duplicate\n"); return;
    }
    printk("PASS: duplicate detection\n");

    if (vfd_register("/dev/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                     rw_buf, 1, 0) != FD_INVALID) {
        printk("FAIL: path too long\n"); return;
    }
    printk("PASS: path length guard\n");

    printk("=== vfd_test PASSED ===\n");
}

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
    fd_init(fs);
    if (process_init(fs)) {
        puts("Failed to initialize processes\n");
        fat_fs_fini(fs);
        disk_fini(disk);
        return;
    }

    for (;;) {
        status = process_spawn(INIT_PATH, NULL, &exit_code);
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
    /* Bytes, 8 per block: bounds how many allocations can be live at once */
    size_t registry_capacity = 1000 * 8;

    kalloc_start(
        heap_start,
        heap_size,
        registry_start,
        registry_capacity
    );

    keyboard_start(100);

    vfd_test();

    init_start();

    stop();
}
