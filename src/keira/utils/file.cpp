#include "file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <lilka/fileutils.h>

bool fexist(const char* path) {
    struct stat st;
    return (
        (strcmp(path, LILKA_SD_ROOT) == 0) || (strcmp(path, LILKA_SPIFFS_ROOT) == 0) ||
        (strcmp(path, LILKA_SD_ROOT "/") == 0) || (strcmp(path, LILKA_SPIFFS_ROOT "/") == 0) ||
        (strcmp(path, "/") == 0) || (access(path, F_OK) == 0) || (stat(path, &st) == 0) || (errno = 0)
    );
}

long fsize(FILE* fd) {
    if (!fd) return 0;

    // Remember current offset
    long tmpOffset = ftell(fd);

    fseek(fd, 0, SEEK_END);

    long fsize = ftell(fd);

    fsize = (fsize < 0) ? 0 : fsize;

    // restore offset
    fseek(fd, tmpOffset, SEEK_SET);

    return fsize;
}

long lendir(DIR* dirfd) {
    if (!dirfd) return 0;

    // Remember current offset
    long tmpOffset = telldir(dirfd);

    if (tmpOffset < 0) {
        return 0;
    }

    long dirLength = 0;

    // Move to dir start
    rewinddir(dirfd);

    // Count entries
    while ((readdir(dirfd)) != NULL)
        dirLength++;

    // restore dir  offset
    seekdir(dirfd, tmpOffset);

    return dirLength;
}
