#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <esp_vfs.h>
#include <soc/soc_caps.h>
#include <lilka/fileutils.h>
#include <sys/stat.h>
#include <lilka/serial.h>
#include <limits.h>

// GLOBAL Storage for both PATH and as a mini buffer for file parts
char posix_fbuf[SOC_CPU_CORES_NUM][PATH_MAX + 1];

bool fexist(const char* path) {
    struct stat st;
    // cppcheck-suppress-begin knownConditionTrueFalse
    return (
        (strcmp(path, LILKA_SD_ROOT) == 0) || (strcmp(path, LILKA_SPIFFS_ROOT) == 0) ||
        (strcmp(path, LILKA_SD_ROOT "/") == 0) || (strcmp(path, LILKA_SPIFFS_ROOT "/") == 0) ||
        (strcmp(path, "/") == 0) || (access(path, F_OK) == 0) || (stat(path, &st) == 0) || (errno = 0)
    );
    // cppcheck-suppress-end knownConditionTrueFalse
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
    char* buf = posix_fbuf[xPortGetCoreID()];
    strncpy(buf, path, PATH_MAX);
    buf[PATH_MAX] = '\0';

    // Skip root '/<mount+point>'
    char* p = strchr(buf + 1, '/');
    if (!p) return 0;

    // Create dirs
    for (;; p++) {
        char* sep = strchr(p, '/');
        if (sep) *sep = '\0';
        if (mkdir(buf, mode) != 0 && errno != EEXIST) return -1;

        if (!sep) break;
        *sep = '/';
        p = sep;
    }
    // clear errno
    errno = 0;
    return 0;
}

void rmpath(const char* path) {
    char* buf = posix_fbuf[xPortGetCoreID()];
    strcpy(buf, path);

    while (strlen(buf) >= strlen(path)) {
        DIR* dir = opendir(buf);
        if (!dir) {
            unlink(buf);

            /* backtrack */
            char* p = strrchr(buf, '/');
            if (!p || p == buf) return;
            *p = '\0';
            continue;
        }

        const struct dirent* de;
        struct stat st;
        int found = 0;

        while ((de = readdir(dir))) {
            if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;

            size_t len = strlen(buf);
            buf[len] = '/';
            strcpy(buf + len + 1, de->d_name);

            if (stat(buf, &st) == 0 && S_ISDIR(st.st_mode)) {
                found = 1;
                break;
            }

            unlink(buf);
            buf[len] = '\0';
        }

        closedir(dir);

        if (!found) {
            rmdir(buf);

            /* backtrack */
            char* p = strrchr(buf, '/');
            if (!p || p == buf) return;
            *p = '\0';
        }
    }
}

String freadstr(FILE* fd) {
    char* buf = posix_fbuf[xPortGetCoreID()];
    String content = "";
    while (!feof(fd)) {
        // yeah, a bit strange length here, theoretically we've to use block size per file system
        // but it won't affect a lot, except file is very very huge
        size_t bytes = fread(buf, 1, PATH_MAX, fd);
        if (bytes == 0) break;
        buf[bytes] = '\0';
        content += buf;
    }
    return content;
}