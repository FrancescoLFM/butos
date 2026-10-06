#include <include/syscalls.h>

/* Exercises the file syscalls on the FAT32 root directory */
#define TEST_PATH   "/test.txt"
#define KEEP_PATH   "/keep.txt"
/* Bigger than a cluster, so the cluster chain has to grow */
#define DATA_SIZE   5000

static char data[DATA_SIZE];
static char buffer[DATA_SIZE];
static int failures = 0;

static void check(const char *what, int got, int expected)
{
    if (got == expected) {
        printf_syscall("filetest: %s ok\n", what);
    } else {
        printf_syscall("filetest: %s FAILED (got %d, expected %d)\n", what, got, expected);
        failures++;
    }
}

static int same(const char *a, const char *b, int n)
{
    for (int i = 0; i < n; i++)
        if (a[i] != b[i])
            return 0;
    return 1;
}

int main()
{
    int fd;
    static char keep[] = "written by butos\n";

    for (int i = 0; i < DATA_SIZE; i++)
        data[i] = 'a' + i % 26;

    /* Leftovers of a previous run */
    remove_syscall(TEST_PATH);
    remove_syscall(KEEP_PATH);

    check("create", create_syscall(TEST_PATH), FILE_OK);
    check("create existing", create_syscall(TEST_PATH), FILE_EXISTS);
    check("create bad name", create_syscall("/toolongname.txt"), FILE_INVALID);
    check("open missing", open_syscall("/missing.txt"), FILE_NOT_FOUND);

    fd = open_syscall(TEST_PATH);
    check("open", fd >= 0, 1);
    check("empty read", read_syscall(fd, buffer, 10), 0);
    check("write", write_syscall(fd, data, DATA_SIZE), DATA_SIZE);
    check("size", seek_syscall(fd, 0, SEEK_END), DATA_SIZE);
    check("seek past end", seek_syscall(fd, 1, SEEK_END), FILE_INVALID);
    check("rewind", seek_syscall(fd, 0, SEEK_SET), 0);
    check("read", read_syscall(fd, buffer, DATA_SIZE), DATA_SIZE);
    check("read data", same(data, buffer, DATA_SIZE), 1);
    check("remove open", remove_syscall(TEST_PATH), FILE_BUSY);
    check("close", close_syscall(fd), FILE_OK);
    check("close twice", close_syscall(fd), FILE_INVALID);

    fd = open_syscall(TEST_PATH);
    check("reopen", fd >= 0, 1);
    check("reopen size", seek_syscall(fd, 0, SEEK_END), DATA_SIZE);
    check("seek middle", seek_syscall(fd, -100, SEEK_CUR), DATA_SIZE - 100);
    check("read tail", read_syscall(fd, buffer, DATA_SIZE), 100);
    check("tail data", same(data + DATA_SIZE - 100, buffer, 100), 1);
    check("overwrite", (seek_syscall(fd, 0, SEEK_SET), write_syscall(fd, "XYZ", 3)), 3);
    check("close", close_syscall(fd), FILE_OK);

    fd = open_syscall(TEST_PATH);
    check("overwrite data", (read_syscall(fd, buffer, 4), same(buffer, "XYZd", 4)), 1);
    close_syscall(fd);

    check("remove", remove_syscall(TEST_PATH), FILE_OK);
    check("open removed", open_syscall(TEST_PATH), FILE_NOT_FOUND);

    /* Left on the disk to be checked from the host */
    check("create keep", create_syscall(KEEP_PATH), FILE_OK);
    fd = open_syscall(KEEP_PATH);
    check("write keep", write_syscall(fd, keep, sizeof(keep) - 1), sizeof(keep) - 1);
    close_syscall(fd);

    printf_syscall("filetest: %d failures\n", failures);
    return failures;
}
