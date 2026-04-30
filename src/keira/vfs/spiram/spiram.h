#pragma once

//////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
//////////////////////////////////////////////////////////////////////////////
// SPIRamVFS implementation of filesystem in external ram
//////////////////////////////////////////////////////////////////////////////

#include "keira/vfs/vfs.h"
#include <vector>
#include "keira/crypto/fnv-1a-32.h"

#include <limits.h> // PATH_MAX

// TODO: to be moved in REG_VFS macro
#define LILKA_TMP_ROOT "/tmp"

//==========================================================
// (-_-)==\~ DEBUG
//==========================================================
// Uncomment this line to get debug information
//==========================================================
// #define KEIRA_SPIRAM_VFS_DEBUG
//==========================================================

// TODO: add quota for max memory usage

#ifdef KEIRA_SPIRAM_VFS_DEBUG
#    define KSVFS_DBG if (1)
#else
#    define KSVFS_DBG if (0)
#endif
//------------------------------------------------ (^_^)==\~

//==========================================================
// (o_O)==\~ CONFIGURATION
//==========================================================
// Count available file descriptors
//==========================================================
#ifndef VFS_SPIRAM_MAX_FD
#    define VFS_SPIRAM_MAX_FD 16
#endif
//==========================================================
// Max filename length [including null terminator]
//==========================================================
#ifndef VFS_SPIRAM_NAME_MAX
#    define VFS_SPIRAM_NAME_MAX 32
#endif
//==========================================================
// Block size used for file storage
//==========================================================
#ifndef VFS_SPIRAM_BLOCK_SIZE
#    define VFS_SPIRAM_BLOCK_SIZE 512
#endif
//==========================================================
// Amount of space in spiram reserved for other purposes
//==========================================================
#ifndef VFS_SPIRAM_RESERVED_SPACE
#    define VFS_SPIRAM_RESERVED_SPACE 1 * 1024 * 1024 // 1MB
#endif
//==========================================================
// Crypto algorithm used for fast node search
//==========================================================
#define VFS_SPIRAM_HASH_INITAL                         FNV_1A_INITIAL_HVAL
#define VFS_SPIRAM_NAME_HASH(BLOCK, SIZE, PARENT_HASH) fnv_32a_buf(BLOCK, SIZE, PARENT_HASH)
//==========================================================
// Default file/dir modes
//==========================================================
#define VFS_SPIRAM_DIR_MODE_DEFAULT  (S_IFDIR | 0755)
#define VFS_SPIRAM_FILE_MODE_DEFAULT (S_IFREG | 0644)
//------------------------------------------------ (^_^)==\~

// TODO: try to use SPIRamVFS instead of RootVFS

//==========================================================
// File block, all files consist from them
//==========================================================
typedef struct {
    uint8_t data[VFS_SPIRAM_BLOCK_SIZE];
} SPIRamFileBlock;
//------------------------------------------------ (^_^)==\~

//==========================================================
// File node on a filesystem
//==========================================================
typedef struct SPIRamNode SPIRamNode;
struct SPIRamNode {
    //======================================================
    // Metadata
    //======================================================
    // basename of file
    //======================================================
    char name[VFS_SPIRAM_NAME_MAX] = "";
    //======================================================
    // Hash used for fast node search
    //======================================================
    uint32_t nhash = 0;
    //======================================================
    // access mode [0755/0644/etc.]
    //======================================================
    mode_t mode;
    //======================================================
    // timestamps
    //======================================================
    time_t atime, mtime, ctime;
    //======================================================
    // count of active file descriptors associated with
    // current node
    //======================================================
    uint32_t refcount = 0;
    //======================================================
    // kind a deletion flag, but actual cleanup happens
    // when (unlinked == true && refcount == 0) in close()
    //======================================================
    bool unlinked = false;
    //------------------------------------------- (^_^)==\~~

    //======================================================
    // Directory tree [linked list]
    //======================================================
    // parent dir node
    //======================================================
    SPIRamNode* pParent = NULL;
    //======================================================
    // next entry in current dir
    //======================================================
    SPIRamNode* pNext = NULL;
    //======================================================
    // ptr to a first entry in case file type is dir
    //======================================================
    SPIRamNode* pChild = NULL;
    //------------------------------------------------------
    //            [pParent]
    //               |        \
    //            [Node]---------->[pNext]
    //               |
    //            [pChild]
    //------------------------------------------- (^_^)==\~~

    //======================================================
    // File data
    //======================================================
    // A vector of fileblocks with actual file bytes
    //======================================================
    std::vector<SPIRamFileBlock*> blocks;
    //======================================================
    // File size, we need it cause we're block aligned
    //======================================================
    size_t fsize = 0;
    //------------------------------------------- (^_^)==\~~
};
//------------------------------------------------ (^_^)==\~

//==========================================================
// File descriptor data
//==========================================================
typedef struct {
    //------------------------------------------------------
    // pointer to file node
    //------------------------------------------------------
    SPIRamNode* pNode;
    //------------------------------------------------------
    // offset used by fseek()/ftell()/fseekdir()/ftelldir()
    //------------------------------------------------------
    long offset;
    //------------------------------------------------------
    // O_RDONLY, O_WRONLY, O_APPEND etc
    //------------------------------------------------------
    int flags;
    //------------------------------------------------------
} SPIRamFileDescriptor;
//------------------------------------------------ (^_^)==\~

//==========================================================
// Directory descriptor data [to be passed as kvfs_dir_t]
//==========================================================
typedef struct {
    //------------------------------------------------------
    // ESP-IDF maintained header
    //------------------------------------------------------
    DIR dir;
    //------------------------------------------------------
    // current dir node
    //------------------------------------------------------
    SPIRamNode* pNode;
    //------------------------------------------------------
    // current position for telldir/seekdir
    // Practically that's a pointer to pNode->pChild
    //------------------------------------------------------
    SPIRamNode* offset;
} SPIRamDir;
//------------------------------------------------ (^_^)==\~

//==========================================================
// SPIRamVFS - filesystem in external RAM
//==========================================================
class SPIRamVFS : public KeiraVFS {
private:
    //======================================================
    // FAPI implementation [ # > man 2 <method_name> ]
    //======================================================
    ssize_t write(int fd, void* src, size_t size) override;
    //------------------------------------------------------
    off_t lseek(int fd, off_t offset, int mode) override;
    //------------------------------------------------------
    ssize_t read(int fd, void* dst, size_t size) override;
    //------------------------------------------------------
    int open(const char* path, int flags, int mode) override;
    //------------------------------------------------------
    int close(int fd) override;
    //------------------------------------------------------
    int fstat(int fd, struct stat* st) override;
    //------------------------------------------------------
#ifdef CONFIG_VFS_SUPPORT_DIR
    int stat(const char* path, struct stat* st) override;
    //------------------------------------------------------
    int unlink(const char* path) override;
    //------------------------------------------------------
    int rename(const char* src, const char* dst) override;
    //------------------------------------------------------
    kvfs_dir_t* opendir(const char* name) override;
    //------------------------------------------------------
    struct dirent* readdir(kvfs_dir_t* pdir) override;
    //------------------------------------------------------
    long telldir(kvfs_dir_t* pdir) override;
    //------------------------------------------------------
    void seekdir(kvfs_dir_t* pdir, long offset) override;
    //------------------------------------------------------
    int closedir(kvfs_dir_t* pdir) override;
    //------------------------------------------------------
    int mkdir(const char* name, mode_t mode) override;
    //------------------------------------------------------
    int rmdir(const char* name) override;
    //------------------------------------------------------
    int access(const char* path, int amode) override;
#endif
    //------------------------------------------- (^_^)==\~~

    //======================================================
    // Node operations
    //======================================================
    // Seeks node in a tree by given @path
    //------------------------------------------------------
    SPIRamNode* findNode(const char* path, SPIRamNode* pStart = NULL);
    //------------------------------------------------------
    // Creates node and attaches it to a directory tree
    //------------------------------------------------------
    SPIRamNode* createNode(const char* name, mode_t mode, SPIRamNode* pParent); //ok
    //------------------------------------------------------
    // Fills stat struct from pNode
    //------------------------------------------------------
    void fillNodeStat(const SPIRamNode* pNode, struct stat* st);
    //------------------------------------------------------
    // Destroys node, detaches from directory tree
    //------------------------------------------------------
    void deleteNode(SPIRamNode* pNode); // + - ok
    //------------------------------------------- (^_^)==\~~

    //======================================================
    // File descriptor operations
    //======================================================
    // Allocating file descriptor associated with node
    //------------------------------------------------------
    int allocfd(SPIRamNode* pNode, int flags);
    //======================================================
    // Misc helper routines
    //======================================================
    // Helper used to calculate node hash from full path, or part of it
    //------------------------------------------------------
    static uint32_t spiram_vfs_hash(const char* rpath, uint32_t parentHash = VFS_SPIRAM_HASH_INITAL);
    //======================================================
    // Special memory allocator working with quota
    //------------------------------------------------------
    static void* spiram_vfs_mem_alloc(size_t size);
    //------------------------------------------- (^_^)==\~~
    //======================================================
    // Storage
    //======================================================
    // Root node
    //------------------------------------------------------
    SPIRamNode* pRoot = NULL;
    //------------------------------------------------------
    // Statically allocated file descriptors used to track
    // file openings
    //
    // 1 KB for each 32 entries
    // NOTE: it seems there's no real reason to make from it
    // vector since we similar way depend on MAX_FDS from
    // esp-idf
    //------------------------------------------------------
    SPIRamFileDescriptor pfds[VFS_SPIRAM_MAX_FD];
    //------------------------------------------- (^_^)==\~~
    //======================================================
    // static buffers for path manipulations
    //======================================================
    // used in open()
    char dirBuffer[PATH_MAX] = {};
    // used in findNode()
    char buffToken[VFS_SPIRAM_NAME_MAX] = {};
    //------------------------------------------- (^_^)==\~~

public:
    explicit SPIRamVFS(const char* mountPoint);
    ~SPIRamVFS();
};
//------------------------------------------------ (^_^)==\~
