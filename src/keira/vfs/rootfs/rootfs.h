#pragma once
//////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
//////////////////////////////////////////////////////////////////////////////
// Keira Root VFS Implementation
//////////////////////////////////////////////////////////////////////////////
#include "keira/kvfs.h"

// Libs
#include <dirent.h>
#include <sys/stat.h>
#include <vector>

// Since esp-idf doesn't allow us to open root directory, we would make it
// possible by own (^_^)==\~

// Threat it as well as an example of how we can add various filesystems
// support in KeiraOS
// It won't actually support Arduino-style APIs, cause am too lazy to even
// read them, though
// if you know how to add those, go on :D (It should be more simple though)

// Uncomment this line to get some debuging information
//#define ROOT_VFS_DEBUG
#ifdef ROOT_VFS_DEBUG
#    define RVFS_DBG if (1)
#else
#    define RVFS_DBG if (0)
#endif

// TODO: Implement SPIRAMfs (/tmp)

// TODO: Implement ZIPfs    (/zip/sd/somefile.zip/somefile_inside.txt)

// TODO: Implement FTPfs    (/ftp/host/port/)
// needs to somehow to think creative to invent way to hide login/pass and not
// show it in a scheme

// Our File Table entry
typedef struct {
    struct dirent dirent;
    struct stat st;
} rvfs_dirent_t;

class RootVFS : public KeiraVFS {
public:
    // Initializes file system and prepares it for further reg()
    // @param rootDir directory from which esp_idf would pass FAPI calls
    RootVFS(const char* rootDir);

    // Register new virtual path in our filesystem
    // @param path
    void addDir(const char* path);

private:
    // Here we just override needed for our VFS functioning methods, others
    // provided by KeiraVFS abstract filesystem
    int stat(const char* path, struct stat* st) override;

    // Opens directory and returns a pointer to internal directory stream
    // It seems allows us to pass almost everything, cause implementation
    // of dir stream is meant to be opaque to user
    // @param name
    kvfs_dir_t* opendir(const char* name) override;

    // Read current dirent record and increment RootVFSDirStream->offset
    // Note, this increment isn't guaranted to be just +1, and depends on
    // actual filesystem implementation
    // DO NOT apply math on it
    // @returns pointer to struct dirent
    struct dirent* readdir(kvfs_dir_t* pdir) override;

    // Closes currently opened directory stream and frees all possible
    // allocated resources allocated for it
    // @param pdir pointer to directory stream
    int closedir(kvfs_dir_t* pdir) override;
    // dirFDs - > dStreams provided via KeiraVFS;

    // in normal fs we need to keep track on a whole directory tree,
    // have to research how to make it better
    std::vector<rvfs_dirent_t> dirEntries;
};
