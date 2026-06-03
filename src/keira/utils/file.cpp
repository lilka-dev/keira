#include "file.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <esp_vfs.h>

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

int mkpath(const char* path, mode_t mode) {
    // Copy path to per CPU unit storage
    char* buf = mkpath_buf[xPortGetCoreID()];
    strncpy(buf, path, ESP_VFS_PATH_MAX);
    buf[ESP_VFS_PATH_MAX] = '\0';

    // Skip root
    char* p = strchr(buf + 1, '/');
    if (!p) return 0;
    // Skip mount point
    p = strchr(p + 1, '/');
    if (!p) return 0;
    p++;

    // Create dirs
    for (;; p++) {
        // Setup temporary null terminator to make part of path
        char* sep = strchr(p, '/');
        if (sep) *sep = '\0';

        // Skip if error EEXIST
        if (mkdir(buf, mode) != 0 && errno != EEXIST) return -1;

        // Reached end
        if (!sep) break;

        // Restore null terminator path part
        *sep = '/';
        p = sep;
    }
}
