#ifndef VERSION_H
#define VERSION_H

/* Kernel identification returned by the uname syscall */

#define BUTOS_SYSNAME       "Butos"
#define BUTOS_NODENAME      "butos"
#define BUTOS_RELEASE       "0.1.0"
#define BUTOS_MACHINE       "i686"

#define UTS_LEN             32

/* Same layout as struct utsname in programs/include/syscalls.h */
struct utsname {
    char sysname[UTS_LEN];
    char nodename[UTS_LEN];
    char release[UTS_LEN];
    char version[UTS_LEN];
    char machine[UTS_LEN];
};

#endif
