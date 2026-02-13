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
#include <vector>

// Since esp-idf doesn't allow us to open root directory, we would make it
// possible by own (^_^)==\~

// Threat it as well as an example of how we can add various filesystems
// support in KeiraOS
// It won't actually support Arduino-style APIs, cause am too lazy to even
// read them, though
// if you know how to add those, go on :D (It should be more simple though)

// TODO: Implement SPIRAMfs (/tmp)

// TODO: Implement ZIPfs    (/zip/sd/somefile.zip/somefile_inside.txt)

// TODO: Implement FTPfs    (/ftp/host/port/)
// needs to somehow to think creative to invent way to hide login/pass and not
// show it in a scheme

// Directory stream implementation used by RootVFS. Doesn't meant to be used
// somewhere else, and represents any data we want to attach to our a.k.a.
// directory file descryptor
typedef struct {
    long offset;
} RootVFSDirStream;

// Inital dir stream value
#define ROOT_VFS_DIR_STREAM_INITIALIZER {.offset = 0}

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

    // Opens directory and returns a pointer to internal directory stream
    // It seems allows us to pass almost everything, cause implementation
    // of dir stream is meant to be opaque to user
    // @param name
    DIR* opendir(const char* name) override;

    // Read current dirent record and increment RootVFSDirStream->offset
    // Note, this increment isn't guaranted to be just +1, and depends on
    // actual filesystem implementation
    // DO NOT apply math on it
    // @returns pointer to struct dirent
    struct dirent* readdir(DIR* pdir) override;

    // Returns current offset in a directory stream
    // @param pdir pointer to directory stream
    // @returns offset
    long telldir(DIR* pdir) override;

    // Sets current directory stream offset. Note offset should be a number
    // once provided by telldir only, cause doesn't always meant to be an
    // index
    // @param pdir pointer to directory stream
    void seekdir(DIR* pdir, long offset) override;

    // Closes currently opened directory stream and frees all possible
    // allocated resources allocated for it
    // @param pdir pointer to directory stream
    int closedir(DIR* pdir) override;

    std::vector<RootVFSDirStream> dirFDs;
    std::vector<struct dirent> dirEntries;

    std::vector<char*> paths;
};
