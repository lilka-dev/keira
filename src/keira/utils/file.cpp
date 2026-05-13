#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
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

ssize_t fsize(FILE* fd) {
    if (!fd) return -1;

    // Remember current offset
    auto tmpOffset = ftell(fd);

    fseek(fd, 0, SEEK_END);

    ssize_t fsize = ftell(fd);

    // restore offset
    fseek(fd, tmpOffset, SEEK_SET);

    return fsize;
}

ssize_t lendir(DIR* dirfd) {
    if (!dirfd) return -1;

    // Remember current offset
    auto tmpOffset = telldir(dirfd);

    if (tmpOffset == -1) {
        return -1;
    }

    ssize_t dirLength = 0;

    // Move to dir start
    rewinddir(dirfd);

    // Count entries
    while ((readdir(dirfd)) != NULL)
        dirLength++;

    // restore dir  offset
    seekdir(dirfd, tmpOffset);

    return dirLength;
}