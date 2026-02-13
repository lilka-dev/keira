#include "rootfs.h"

// Libs:
#include <stdbool.h>
#include <errno.h>
#include <lilka/serial.h>

#define ROOT_VFS_DEBUG

#ifdef ROOT_VFS_DEBUG
#    define RVFS_DBG if (1)
#else
#    define RVFS_DBG if (0)
#endif

// TODO: implement stat()

RootVFS::RootVFS(const char* rootDir) {
    strcpy(this->rootDir, rootDir);
}

void RootVFS::addDir(const char* path) {
    RVFS_DBG lilka::serial.log("Registering %s int RootVFS\n", path);
    // TODO: allow overrive DT_DIR with DT_FILE or something else
    struct dirent newEntry = {
        .d_ino = static_cast<short unsigned int>((this->dirEntries.size() + 1)), .d_type = DT_DIR
    };
    strcpy(newEntry.d_name, path + 1);
    this->dirEntries.push_back(newEntry);
}

DIR* RootVFS::opendir(const char* name) {
    RVFS_DBG lilka::serial.log("External call to opendir, routed via RootVFS\nPath:%s\n", name);

    // Allow only rootDir open
    if (strcmp(name, "/") == 0) {
        // Create new directory stream
        RootVFSDirStream dirStream = ROOT_VFS_DIR_STREAM_INITIALIZER;
        dirFDs.push_back(dirStream);

        DIR* pdir = reinterpret_cast<DIR*>(&(dirFDs[dirFDs.size() - 1]));
        RVFS_DBG lilka::serial.log("Returning dir stream with offset %d", dirFDs[dirFDs.size() - 1].offset);
        return pdir;
    } else {
        errno = EINVAL;
        return NULL;
    }
}

struct dirent* RootVFS::readdir(DIR* pdir) {
    // Cast to internal directory stream implementation
    RootVFSDirStream* dirStream = reinterpret_cast<RootVFSDirStream*>(pdir);
    RVFS_DBG lilka::serial.log("External call to readdir, routed via RootVFS\nOffset:%d\n", dirStream->offset);

    // It seems that offset is always equal to max dirEntries, what do I miss?
    if (dirStream->offset >= 0 && dirEntries.size() > dirStream->offset) {
        struct dirent* cDirent = &(dirEntries[dirStream->offset]);
        dirStream->offset++;
        return cDirent;
    }

    return NULL;
}

long RootVFS::telldir(DIR* pdir) {
    // Cast to internal directory stream implementation
    RootVFSDirStream* dirStream = reinterpret_cast<RootVFSDirStream*>(pdir);
    return dirStream->offset;
}

void RootVFS::seekdir(DIR* pdir, long offset) {
    // Cast to internal directory stream implementation
    RootVFSDirStream* dirStream = reinterpret_cast<RootVFSDirStream*>(pdir);

    dirStream->offset = offset;
}

int RootVFS::closedir(DIR* pdir) {
    // Cast to internal directory stream implementation
    RootVFSDirStream* dirStream = reinterpret_cast<RootVFSDirStream*>(pdir);

    // Handle NULL ptr
    if (dirStream == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Seek & remove from table openned directory streams
    bool success = false;
    size_t i = 0;
    for (const auto& dFD : dirFDs) {
        if (dirStream == &dFD) {
            dirFDs.erase(dirFDs.begin() + i);
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
