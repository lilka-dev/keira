#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

// Keira Root VFS Implementation

///////////////////////////////////////////////////////////////////////////////////////////////////////

// Since esp-idf doesn't allow us to open root directory, we would make it possible by own (^_^)==\~

// Threat it as well as an example of how we can add various filesystems support in KeiraOS
// It won't actually support Arduino-style APIs, cause am too lazy to even read them, though
// if u know how to add those, go on :D (It should be more simple though)

// TODO: Implement SPIRAMfs (/tmp)

// TODO: Implement ZIPfs    (/zip/sd/somefile.zip/somefile_inside.txt)

// TODO: Implement FTPfs    (/ftp/host/port/)
// needs to somehow to think creative to invent way to hide login/pass and not show it in a scheme

#include <dirent.h>
#include "keira/kvfs.h"

class RootVFS : public KeiraVFS {
public:
    void addDir(const char *path);
private:
    // Here we just override needed for our VFS functioning methods, others
    // provided by KeiraVFS abstract filesystem
    DIR* opendir(const char* name) override;
    struct dirent* readdir(DIR* pdir) override;
    long telldir(DIR* pdir) override;
    void seekdir(DIR* pdir, long offset) override;
    int closedir(DIR* pdir) override;

};
