#pragma once
//////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
//////////////////////////////////////////////////////////////////////////////
// Keira VFS(Virtual Filesystem) abstract wrapper to implement VFSs
//////////////////////////////////////////////////////////////////////////////

// ESP-IDF VFS implementation
#include <esp_vfs.h>

// Standard libraries
#include <vector>
#include <cstddef>
#include <errno.h>

//////////////////////////////////////////////////////////////////////////////
// Default file api implementations. this is kind a funky, but theoretically
// we can provide even debug for calls using __PRETTY_FUNCTION__ here, but
// let's save it for better times (^__^) == \~
// TODO: maybe hide it in keira/bits/kvfs.h ? :D
#define DEFAULT_FAPI_IMPL \
    {                     \
        errno = ENOSYS;   \
        return -1;        \
    }
#define DEFAULT_PFAPI_IMPL \
    {                      \
        errno = ENOSYS;    \
        nullptr;           \
    }
//////////////////////////////////////////////////////////////////////////////

// Abstract class for VFS(Virtual Filesystem).
// Each VFS implementation must overload needed methods specified here.
// All others would use default implementations provided via this abstract
// class.
// After that, it should be registered in ksystem->regiserVFS()
// except unusual case of RootVFS which might have a need to be registered
// other way.
// Keep in mind that current approach doesn't provide Arduino-like methods
// to work with those files, but this can change in future.
// So, if possible, just stick to a well documented/tested/used POSIX file api
class KeiraVFS {
private:
    // VFS Implementation routines ===========================================
    // Warning : Methods below should be in esp_vfs_t order, all other virtual
    // methods should be defined after, and VFS implementations should specify
    // this class first due to vtable manipulations in KeiraVFS->reg()
    // =======================================================================
    virtual ssize_t write(int fd, void* dst, size_t size) DEFAULT_FAPI_IMPL;
    virtual off_t lseek(int fd, off_t size, int mode) DEFAULT_FAPI_IMPL;
    virtual ssize_t read(int fd, void* dst, size_t size) DEFAULT_FAPI_IMPL;
    virtual ssize_t pread(int fd, void* dst, size_t size, off_t offset) DEFAULT_FAPI_IMPL;
    virtual ssize_t pwrite(int fd, const void* src, size_t size, off_t offset) DEFAULT_FAPI_IMPL;
    virtual int open(const char* path, int flags, int mode) DEFAULT_FAPI_IMPL;
    virtual int close(int fd) DEFAULT_FAPI_IMPL;
    virtual int fstat(int fd, struct stat* st) DEFAULT_FAPI_IMPL;

#ifdef CONFIG_VFS_SUPPORT_DIR
    virtual int stat(const char* path, struct stat* st) DEFAULT_FAPI_IMPL;
    virtual int link(const char* n1, const char* n2) DEFAULT_FAPI_IMPL;
    virtual int unlink(const char* path) DEFAULT_FAPI_IMPL;
    virtual int rename(const char* src, const char* dst) DEFAULT_FAPI_IMPL;
    virtual DIR* opendir(const char* name) DEFAULT_PFAPI_IMPL;
    virtual struct dirent* readdir(DIR* pdir) DEFAULT_PFAPI_IMPL;
    virtual long telldir(DIR* pdir) DEFAULT_FAPI_IMPL;
    virtual void seekdir(DIR* pdir, long offset) DEFAULT_PFAPI_IMPL;
    virtual int closedir(DIR* pdir) DEFAULT_FAPI_IMPL;
    virtual int mkdir(const char* name, mode_t mode) DEFAULT_FAPI_IMPL;
    virtual int rmdir(const char* name) DEFAULT_FAPI_IMPL;
#endif

    virtual int fcntl(int fd, int cmd, int arg) DEFAULT_FAPI_IMPL;
    virtual int ioctl(int fd, int cmd, va_list args) DEFAULT_FAPI_IMPL;
    virtual int fsync(int fd) DEFAULT_FAPI_IMPL;

#ifdef CONFIG_VFS_SUPPORT_DIR
    virtual int access(const char* path, int amode) DEFAULT_FAPI_IMPL;
    virtual int truncate(const char* path, off_t length) DEFAULT_FAPI_IMPL;
    virtual int ftruncate(int fd, off_t length) DEFAULT_FAPI_IMPL;
    virtual int utime(char* path, const struct utimbuf* times) DEFAULT_FAPI_IMPL;
#endif

#ifdef CONFIG_VFS_SUPPORT_TERMIOS
    virtual int tcsetattr(int fd, int optional_actions, const struct termios* p) DEFAULT_FAPI_IMPL;
    virtual int tcgetattr(int fd, struct termios p) DEFAULT_FAPI_IMPL;
    virtual int tcdrain(int fd) DEFAULT_FAPI_IMPL;
    virtual int tcflush(int fd, int select) DEFAULT_FAPI_IMPL;
    virtual int tcflow(int fd, int action) DEFAULT_FAPI_IMPL;
    virtual pid_t tcgetsid(int fd) DEFAULT_FAPI_IMPL;
    virtual int tcsendbreak(int fd, int duration) DEFAULT_FAPI_IMPL;
#endif

// nah, this part seems to be unfinished even in esp-idf, cause
// there's no implementations whose allow to pass context pointer
#ifdef CONFIG_VFS_SUPPORT_SELECT
#endif
    // VFS Implementation END ================================================
    // Place to define other virtual methods in case we ever need them

    char rootDir[ESP_VFS_PATH_MAX] = {};

    // File/Dir descriptors in use
    //    std::vector<DIR*> oDirs;
    //    std::vector<FILE * oFiles>;

public:
    KeiraVFS(const char* path);
    // Registers VFS implementation
    void reg();
    // Unregisters VFS implementation
    void unreg();

    // local storage for file/dirs fds

    bool registered = false;
};
