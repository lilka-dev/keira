#include "keira/vfs/vfs.h"
//////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
//////////////////////////////////////////////////////////////////////////////
// SPIRamVFS implementation of filesystem in external ram
//////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "keira/crypto/fnv-1a-32.h"

// TODO : to be moved into keira/utils/mem/allocators
#include "keira/keira_allocators.h"

// TODO: to be moved in REG_VFS macro
#define LILKA_TMP_ROOT "/tmp"

//==========================================================
// (-_-)==\~ DEBUG
//==========================================================
// Uncomment this line to get debug information
//----------------------------------------------------------
#define KEIRA_SPIRAM_VFS_DEBUG
//----------------------------------------------------------
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
//----------------------------------------------------------
#ifndef VFS_SPIRAM_MAX_FD
#    define VFS_SPIRAM_MAX_FD 16
#endif
//----------------------------------------------------------
// Max filename length [including null terminator]
//----------------------------------------------------------
#ifndef VFS_SPIRAM_NAME_MAX
#    define VFS_SPIRAM_NAME_MAX 32
#endif
//----------------------------------------------------------
// Block size used for file storage
//----------------------------------------------------------
#ifndef VFS_SPIRAM_BLOCK_SIZE
#    define VFS_SPIRAM_BLOCK_SIZE 512
#endif
//----------------------------------------------------------
// Crypto algorithm used for fast node search
//----------------------------------------------------------
#define VFS_SPIRAM_NAME_HASH(BLOCK, SIZE) fnv_32a_buf(BLOCK, SIZE, FNV_1A_INITIAL_HVAL)
//------------------------------------------------ (^_^)==\~

// TODO: try to use SPIRamVFS instead of RootVFS

//==========================================================
// File block, all files consist from them
//==========================================================
typedef struct {
    uint8_t data[VFS_SPIRAM_BLOCK_SIZE];
} SPIRamFileBlock;
//------------------------------------------------ (^_^)==\~

// TODO: reuse someting for that
typedef enum { SPIRAM_DIR, SPIRAM_FILE } SPIRamNodeType;

//==========================================================
// File node on a filesystem
//==========================================================
typedef struct SPIRamNode SPIRamNode;
struct SPIRamNode {
    //======================================================
    // Metadata
    //======================================================
    // basename of file
    //------------------------------------------------------
    char name[VFS_SPIRAM_NAME_MAX];
    //------------------------------------------------------
    SPIRamNodeType type;
    //------------------------------------------------------
    // Hash used for fast node search
    //------------------------------------------------------
    uint32_t nhash;
    //------------------------------------------------------
    // access mode
    //------------------------------------------------------
    mode_t mode;
    //------------------------------------------------------
    // timestamps
    //------------------------------------------------------
    time_t atime, mtime, ctime;
    //------------------------------------------------------
    // count of active file descriptors associated with
    // current node
    //------------------------------------------------------
    uint32_t refcount;
    //------------------------------------------------------
    // kind a deletion flag, but actual cleanup happens
    // when (unlinked == true && refcount == 0) in close()
    //------------------------------------------------------
    bool unlinked = false;
    //------------------------------------------- (^_^)==\~~

    //======================================================
    // Directory tree [linked list]
    //======================================================
    // parent dir node
    //------------------------------------------------------
    SPIRamNode* pParent;
    //------------------------------------------------------
    // next entry in current dir
    //------------------------------------------------------
    SPIRamNode* pNext;
    //------------------------------------------------------
    // ptr to a first entry in case file type is dir
    //------------------------------------------------------
    SPIRamNode* pChildren;
    //------------------------------------------- (^_^)==\~~

    //======================================================
    // File data
    //======================================================
    // A vector of fileblocks with actual file bytes
    //------------------------------------------------------
    std::vector<SPIRamFileBlock, SPIRamAllocator<SPIRamFileBlock>> blocks;
    //------------------------------------------------------
    // File size, we need it cause we're block aligned
    //------------------------------------------------------
    size_t fsize;
    //------------------------------------------- (^_^)==\~~
};
//------------------------------------------------ (^_^)==\~

//==========================================================
// File descriptor data
//==========================================================
typedef struct {
    // indicates that file descriptor is used
    // TODO: is it atomic?
    bool inUse;
    //------------------------------------------------------
    // pointer to file node
    //------------------------------------------------------
    SPIRamNode* pNode;
    //------------------------------------------------------
    // cursor used by fseek()/ftell()/fseekdir()/ftelldir()
    //------------------------------------------------------
    long cursor;
    //------------------------------------------------------
    // O_RDONLY, O_WRONLY, O_APPEND etc
    //------------------------------------------------------
    int flags;
    //------------------------------------------------------
} SPIRamFileDescriptor;
//------------------------------------------------ (^_^)==\~

//==========================================================
// SPIRamVFS - filesystem in external RAM
//==========================================================
class SPIRamVFS : public KeiraVFS {
private:
    //======================================================
    // FAPI implementation [ # > man 2 <method_name> ]
    //======================================================
    ssize_t write(int fd, void* dst, size_t size) override;
    //------------------------------------------------------
    off_t lseek(int fd, off_t size, int mode) override;
    //------------------------------------------------------
    ssize_t read(int fd, void* dst, size_t size) override;
    //------------------------------------------------------
    int open(const char* path, int flags, int mode) override;
    //------------------------------------------------------
    int close(int fd) override;
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
#endif
    //------------------------------------------- (^_^)==\~~

    //======================================================
    // Node operations
    //======================================================
    // Seeks node in a tree by given @path
    //------------------------------------------------------
    SPIRamNode* findNode(SPIRamNode* pStart, const char* path);
    //------------------------------------------------------
    // Creates node and attaches it to a directory tree
    //------------------------------------------------------
    SPIRamNode* createNode(SPIRamNode* pParent, const char* name, SPIRamNodeType type);
    //------------------------------------------------------
    // Destroys node, detaches from directory tree
    //------------------------------------------------------
    void deleteNode(SPIRamNode* pNode);
    //------------------------------------------- (^_^)==\~~

    //======================================================
    // File descriptor operations
    //======================================================
    // Allocating file descriptor associated with node
    //------------------------------------------------------
    int allocFd(SPIRamNode* pNode, int flags);
    //------------------------------------------------------
    bool checkFd(int fd);
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
    //------------------------------------------------------
    SPIRamFileDescriptor pfds[VFS_SPIRAM_MAX_FD];
    //------------------------------------------- (^_^)==\~~

public:
    explicit SPIRamVFS(const char* mountPoint);
    ~SPIRamVFS();
};
//------------------------------------------------ (^_^)==\~
