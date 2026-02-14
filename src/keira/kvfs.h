#pragma once
//////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
//////////////////////////////////////////////////////////////////////////////
// Keira VFS(Virtual Filesystem) abstract wrapper to implement VFSs
//////////////////////////////////////////////////////////////////////////////
// Abstract class for VFS(Virtual Filesystem).
// Each VFS implementation must overload needed methods specified here.
// All others would use default implementations provided via this abstract
// class.
// After that, it should be registered in ksystem except unusual case of
// RootVFS which might have a need to be registered other way. Look
// keira/ksystem.h for details

// Keep in mind that current approach doesn't provide Arduino-like methods
// to work with those files, but this can change in future.
// So, if possible, just stick to a well documented/tested/used POSIX file api

// ESP-IDF VFS implementation
#include <esp_vfs.h>

// Standard libraries
#include <vector>
#include <cstddef>
#include <errno.h>
#include <lilka/serial.h>

// Uncomment this line to get some additional debuging information through
// serial
//#define KEIRA_VFS_DEBUG

#ifdef KEIRA_VFS_DEBUG
#    define KVFS_DBG if (1)
#else
#    define KVFS_DBG if (0)
#endif

// Default file api implementations.
#define DEFAULT_FAPI_IMPL                                                                   \
    {                                                                                       \
        KVFS_DBG lilka::serial.log("[KVFS]Runing not implemented %s", __PRETTY_FUNCTION__); \
        errno = ENOSYS;                                                                     \
        return -1;                                                                          \
    }

#define DEFAULT_PFAPI_IMPL                                                                  \
    {                                                                                       \
        KVFS_DBG lilka::serial.log("[KVFS]Runing not implemented %s", __PRETTY_FUNCTION__); \
        errno = ENOSYS;                                                                     \
        return nullptr;                                                                     \
    }
#define DEFAULT_VFAPI_IMPL                                                                  \
    {                                                                                       \
        KVFS_DBG lilka::serial.log("[KVFS]Runing not implemented %s", __PRETTY_FUNCTION__); \
        errno = ENOSYS;                                                                     \
    }

// Implementation provided by KeiraVFS
#define KVFS_IMPL ;

// This is our analogue to DIR, simplyfying some stuff. DIR isn't meant to be
// used by user, so we  can use it almost any way we want.
// You can specify any other datatype by own as well, but keep in mind that
// you've to provide ESP-IDF maintained header in it. Though, this one also
// allows to pass any data you may want inside through @data field.
// Note in case own datatype, you have to override KVFS_IMPL methods as well
typedef struct {
    // ESP-IDF maintained header. I believe it's a number representing index
    // of current filsystem registration
    DIR dir;
    // Offset representing a position inside directory stream
    long offset;
    // Place to incorporate any additional data you may want to use inside
    // directory stream
    void* data;
} kvfs_dir_t;

// Default kvfs_dir_t(a.k.a DIR) initializer
#define KVFS_DIR_INITIALIZER {.dir = {}, .offset = 0, .data = NULL}

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
    virtual kvfs_dir_t* opendir(const char* name) DEFAULT_PFAPI_IMPL;
    virtual struct dirent* readdir(kvfs_dir_t* pdir) DEFAULT_PFAPI_IMPL;
    virtual int readdir_r(kvfs_dir_t* pdir, struct dirent* entry, struct dirent** out_dirent) DEFAULT_FAPI_IMPL;

    virtual long telldir(kvfs_dir_t* pdir) KVFS_IMPL;
    virtual void seekdir(kvfs_dir_t* pdir, long offset) KVFS_IMPL;
    virtual int closedir(kvfs_dir_t* pdir) DEFAULT_FAPI_IMPL;
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
protected:
    // Root directory used by our filesystem
    char rootDir[ESP_VFS_PATH_MAX] = {};
    // Directory streams in use for current filesystem
    // You have to fill it on opendir(), and clean on closedir()
    std::vector<kvfs_dir_t> dirStreams;

public:
    KeiraVFS() {}; // empty canstructor
    KeiraVFS(const char* path);
    // Registers VFS implementation
    void reg();
    // Unregisters VFS implementation
    void unreg();

    // local storage for file/dirs fds

    bool registered = false;
};
