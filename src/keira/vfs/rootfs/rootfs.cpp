#include "rootfs.h"

// Libs:
#include <stdbool.h>
#include <errno.h>
#include <lilka/serial.h>

RootVFS::RootVFS(const char* rootDir) {
    strcpy(this->rootDir, rootDir);
}

void RootVFS::addDir(const char* path) {
    // TODO: allow overrive DT_DIR with DT_FILE or something else
    rvfs_dirent_t newEntry = {
        .dirent =
            {

                .d_ino = static_cast<short unsigned int>((this->dirEntries.size() + 1)), .d_type = DT_DIR
            },
        .st = {.st_mode = S_IFDIR}
    };
    strcpy(newEntry.dirent.d_name, basename(path));
    RVFS_DBG lilka::serial.log("[RVFS] %s new entry %s", __PRETTY_FUNCTION__, newEntry.dirent.d_name);
    this->dirEntries.push_back(newEntry);
}

// This part has not much sense now, see a point, if it appears to be
// LILKA_SD_SPIFFS for example, then stat routed to it :D
int RootVFS::stat(const char* path, struct stat* st) {
    // TODO: less lazy handle path
    // A bit rusty all of that, but still :)
    for (const auto& dirEntry : dirEntries) {
        RVFS_DBG lilka::serial.log("cmp %s to %s in %s", dirEntry.dirent.d_name, basename(path), __PRETTY_FUNCTION__);
        if (strcmp(dirEntry.dirent.d_name, basename(path)) == 0) {
            memcpy(st, &(dirEntry.st), sizeof(struct stat));
            return 0;
        }
    }

    return -1;
}

kvfs_dir_t* RootVFS::opendir(const char* name) {
    RVFS_DBG lilka::serial.log("[RVFS] %s Path:%s\n", __PRETTY_FUNCTION__, name);

    // Allow only rootDir open
    if (strcmp(name, "/") == 0) {
        // Create new directory stream
        kvfs_dir_t dStream = KVFS_DIR_INITIALIZER;
        // Push new directory stream to keep track on it
        dirStreams.push_back(dStream);

        kvfs_dir_t* pdir = &(dirStreams[dirStreams.size() - 1]);
        return pdir;
    } else {
        errno = EINVAL;
        return NULL;
    }
}

struct dirent* RootVFS::readdir(kvfs_dir_t* pdir) {
    RVFS_DBG lilka::serial.log("[RVFS] %s offset:%d\n", __PRETTY_FUNCTION__, pdir->offset);

    // It seems that offset is always equal to max dirEntries, what do I miss?
    if (pdir->offset >= 0 && dirEntries.size() > pdir->offset) {
        struct dirent* cDirent = &(dirEntries[pdir->offset].dirent);
        pdir->offset++;
        return cDirent;
    }

    return NULL;
}

int RootVFS::closedir(kvfs_dir_t* pdir) {
    // Handle NULL ptr
    if (pdir == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Seek & remove from table openned directory streams
    bool success = false;
    size_t i = 0;
    for (const auto& dirStream : dirStreams) {
        if (pdir == &dirStream) {
            dirStreams.erase(dirStreams.begin() + i);
            success = true;
            break; // assume no duplicates
        }
        i++;
    }

    // Handle errors
    if (success) return 0;
    else {
        errno = EBADF;
        return -1;
    }
}
