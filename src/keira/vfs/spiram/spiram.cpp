#include "spiram.h"
#include <errno.h>
#include <string.h>
#include <esp_heap_caps.h>

//================================================================================================
// ERROR hangling macro
//================================================================================================
// Count of bytes to return in APIs requirng that, in case of error
#define ERR_BYTES_COUNT -1
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
// File descriptor(fd) specific macro
//================================================================================================
// Determines is fd in a safe range
//------------------------------------------------------------------------------------------------
#define FD_ISVALID(FD) (((FD) >= 0) && ((FD) < VFS_SPIRAM_MAX_FD) && pfds[FD].pNode)
//------------------------------------------------------------------------------------------------
// Returns access mode for given file descriptor
//------------------------------------------------------------------------------------------------
#define FD_ACCMODE(FD) ((pfds[(FD)].flags) & O_ACCMODE)
//------------------------------------------------------------------------------------------------
// Determines if fd allows file writes
//------------------------------------------------------------------------------------------------
#define FD_ISWRITABLE(FD) ((FD_ACCMODE(FD) == O_WRONLY) || (FD_ACCMODE(FD) == O_RDWR))
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
ssize_t SPIRamVFS::write(int fd, void* src, size_t size) {
    // Do the harlem check
    if (!(FD_ISVALID(fd) && FD_ISWRITABLE(fd))) {
        errno = EBADF;
        return ERR_BYTES_COUNT;
    }
    if (!src) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }
    if (!size) return 0;

    SPIRamFileDescriptor& desc = pfds[fd];
    SPIRamNode* pNode = desc.pNode;

    if (S_ISDIR(pNode->mode)) {
        errno = EISDIR;
        return ERR_BYTES_COUNT;
    }

    // Adjust offset
    if (desc.flags & O_APPEND) desc.offset = static_cast<long>(pNode->fsize);

    size_t writeEnd = (size_t)desc.offset + size;
    size_t blocksNeeded = (writeEnd + VFS_SPIRAM_BLOCK_SIZE - 1) / VFS_SPIRAM_BLOCK_SIZE;

    // allocate new file blocks if needed
    while (pNode->blocks.size() < blocksNeeded) {
        SPIRamFileBlock* pBlock = static_cast<SPIRamFileBlock*>(spiram_vfs_mem_alloc(sizeof(SPIRamFileBlock)));
        // Ensure we still have memory
        if (!pBlock) {
            errno = ENOMEM;
            return ERR_BYTES_COUNT;
        }

        memset(pBlock, 0, sizeof(SPIRamFileBlock));

        pNode->blocks.push_back(pBlock);
    }

    // Thanks god we've no need to push data to the right, cause POSIX says we simply overwrite
    // in case offset is located in the middle of file

    size_t remaining = size;
    size_t srcOff = 0;
    size_t pos = (size_t)desc.offset;

    while (remaining > 0) {
        size_t blockIdx = pos / VFS_SPIRAM_BLOCK_SIZE;
        size_t byteIdx = pos % VFS_SPIRAM_BLOCK_SIZE;

        size_t chunk = VFS_SPIRAM_BLOCK_SIZE - byteIdx;
        if (chunk > remaining) chunk = remaining;

        memcpy(&pNode->blocks[blockIdx]->data[byteIdx], reinterpret_cast<const uint8_t*>(src) + srcOff, chunk);

        pos += chunk;
        srcOff += chunk;
        remaining -= chunk;
    }

    // time to change time
    pNode->atime = pNode->mtime = time(NULL);

    // Advance offset and fileSize
    if (writeEnd > pNode->fsize) pNode->fsize = writeEnd;
    desc.offset = static_cast<long>(pos);

    KSVFS_DBG printf("[SPIRamVFS] write(%d, %zu) offset=%ld fsize=%zu\n", fd, size, desc.offset, pNode->fsize);

    return (ssize_t)size;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
off_t SPIRamVFS::lseek(int fd, off_t offset, int mode) {
    if (!FD_ISVALID(fd)) {
        errno = EBADF;
        return ERR_BYTES_COUNT;
    }

    SPIRamFileDescriptor& desc = pfds[fd];
    long newOffset;

    switch (mode) {
        case SEEK_SET:
            newOffset = static_cast<long>(offset);
            break;
        case SEEK_CUR:
            newOffset = desc.offset + static_cast<long>(offset);
            break;
        case SEEK_END:
            newOffset = static_cast<long>(desc.pNode->fsize) + static_cast<long>(offset);
            break;
        default:
            errno = EINVAL;
            return ERR_BYTES_COUNT;
    }

    if (newOffset < 0) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }

    desc.offset = newOffset;
    KSVFS_DBG printf("[SPIRamVFS] lseek(%d, %ld, %d) -> %ld\n", fd, static_cast<long>(offset), mode, newOffset);
    return static_cast<off_t>(newOffset);
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
ssize_t SPIRamVFS::read(int fd, void* dst, size_t size) {
    if (!FD_ISVALID(fd)) {
        errno = EBADF;
        return ERR_BYTES_COUNT;
    }

    if (!dst) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }
    if (!size) return 0;

    SPIRamFileDescriptor& desc = pfds[fd];
    SPIRamNode* pNode = desc.pNode;

    if (S_ISDIR(pNode->mode)) {
        errno = EISDIR;
        return ERR_BYTES_COUNT;
    }
    if ((desc.flags & O_ACCMODE) == O_WRONLY) {
        errno = EBADF;
        return ERR_BYTES_COUNT;
    }
    if ((size_t)desc.offset >= pNode->fsize) return 0;

    size_t available = pNode->fsize - (size_t)desc.offset;
    size_t toRead = (size < available) ? size : available;

    // do read

    size_t remaining = toRead;
    size_t dstOff = 0;
    size_t pos = (size_t)desc.offset;

    while (remaining > 0) {
        size_t blockIdx = pos / VFS_SPIRAM_BLOCK_SIZE;
        size_t byteIdx = pos % VFS_SPIRAM_BLOCK_SIZE;

        size_t chunk = VFS_SPIRAM_BLOCK_SIZE - byteIdx;
        if (chunk > remaining) chunk = remaining;

        memcpy(reinterpret_cast<uint8_t*>(dst) + dstOff, &pNode->blocks[blockIdx]->data[byteIdx], chunk);

        pos += chunk;
        dstOff += chunk;
        remaining -= chunk;
    }

    desc.offset += static_cast<long>(toRead);

    // update access time
    pNode->atime = time(NULL);

    KSVFS_DBG printf("[SPIRamVFS] read(%d, %zu) got=%zu offset=%ld\n", fd, size, toRead, desc.offset);

    return (ssize_t)toRead;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::open(const char* path, int flags, int mode) {
    if (!path) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }

    KSVFS_DBG printf("[SPIRamVFS] open('%s', flags=0x%x)\n", path, flags);

    SPIRamNode* pNode = findNode(path);

    if (pNode && (flags & O_CREAT) && (flags & O_EXCL)) {
        errno = EEXIST;
        return ERR_BYTES_COUNT;
    }

    if (!pNode) {
        if (!(flags & O_CREAT)) {
            errno = ENOENT;
            return ERR_BYTES_COUNT;
        }

        char* dir = dirBuffer;

        strcpy(dir, path);
        const char* lastSlash = strrchr(path, '/');
        const char* basename = lastSlash ? lastSlash + 1 : path;
        SPIRamNode* pParent;

        if (!lastSlash) {
            pParent = pRoot;
        } else {
            dir[lastSlash - path] = '\0';
            pParent = findNode(dir);
        }

        if (!pParent) {
            errno = ENOENT;
            return ERR_BYTES_COUNT;
        }
        pNode = createNode(basename, VFS_SPIRAM_FILE_MODE_DEFAULT, pParent);
        if (!pNode) return ERR_BYTES_COUNT; // errno already set by createNode
    }

    if (S_ISDIR(pNode->mode)) {
        errno = EISDIR;
        return ERR_BYTES_COUNT;
    }

    if (flags & O_TRUNC) {
        // free allocated blocks
        for (auto& block : pNode->blocks)
            heap_caps_free(block);

        pNode->blocks.clear();
        pNode->fsize = 0;

        pNode->atime = pNode->ctime = time(NULL);
    }

    // Allocating new fd
    int fd = allocfd(pNode, flags);

    if (!FD_ISVALID(fd)) return ERR_BYTES_COUNT;

    if (flags & O_APPEND) pfds[fd].offset = static_cast<long>(pNode->fsize);

    return fd;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::close(int fd) {
    if (!FD_ISVALID(fd)) {
        errno = EBADF;
        return ERR_BYTES_COUNT;
    }

    KSVFS_DBG printf("[SPIRamVFS] close(%d)\n", fd);

    SPIRamNode* pNode = pfds[fd].pNode;

    pNode->refcount--;

    SPIRamFileDescriptor* pDesc = &(pfds[fd]);

    memset(pDesc, 0, sizeof(SPIRamFileDescriptor));

    // Deferred deletion: if node was unlinked and no more refs
    if (pNode->unlinked && pNode->refcount == 0) {
        deleteNode(pNode);
    }

    return 0;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::fstat(int fd, struct stat* st) {
    if (!(FD_ISVALID(fd))) {
        errno = EBADF;
        return ERR_BYTES_COUNT;
    }
    if (!st) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }

    SPIRamNode* pNode = pfds[fd].pNode;

    fillNodeStat(pNode, st);

    return 0;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::stat(const char* path, struct stat* st) {
    if (!path || !st) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }

    const SPIRamNode* pNode = findNode(path);
    if (!pNode) {
        errno = ENOENT;
        return ERR_BYTES_COUNT;
    }

    fillNodeStat(pNode, st);
    return 0;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::unlink(const char* path) {
    if (!path) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }

    SPIRamNode* pNode = findNode(path);
    if (!pNode) {
        errno = ENOENT;
        return ERR_BYTES_COUNT;
    }
    if (S_ISDIR(pNode->mode) && pNode->pChild) {
        errno = ENOTEMPTY;
        return ERR_BYTES_COUNT;
    }

    if (pNode->refcount) {
        pNode->unlinked = true;
    } else deleteNode(pNode);

    return 0;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
kvfs_dir_t* SPIRamVFS::opendir(const char* name) {
    SPIRamNode* pNode = findNode(name);
    if (!pNode) {
        errno = ENOENT;
        return NULL;
    }
    if (!S_ISDIR(pNode->mode)) {
        errno = ENOTDIR;
        return NULL;
    }

    SPIRamDir* pDir = new SPIRamDir();
    pDir->offset = pNode->pChild;
    pDir->pNode = pNode;
    pNode->refcount++;

    return (kvfs_dir_t*)pDir;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
struct dirent* SPIRamVFS::readdir(kvfs_dir_t* pdir) {
    if (!pdir) {
        errno = EINVAL;
        return NULL;
    }

    SPIRamDir* d = reinterpret_cast<SPIRamDir*>(pdir);
    SPIRamNode* cur = d->offset;

    if (!cur) return NULL; // end of directory

    // advance to next
    d->offset = cur->pNext;

    // fill dirent
    static struct dirent entry;
    memset(&entry, 0, sizeof(entry));
    entry.d_ino = (ino_t)(uintptr_t)cur;
    entry.d_type = S_ISDIR(cur->mode) ? DT_DIR : DT_REG;
    strncpy(entry.d_name, cur->name, sizeof(entry.d_name) - 1);

    KSVFS_DBG printf("[SPIRamVFS] readdir -> '%s'\n", cur->name);

    return &entry;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::closedir(kvfs_dir_t* pdir) {
    if (!pdir) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }

    SPIRamDir* d = reinterpret_cast<SPIRamDir*>(pdir);
    SPIRamNode* pNode = d->pNode;

    pNode->refcount--;

    delete d;

    if (pNode->unlinked && pNode->refcount == 0) deleteNode(pNode);

    return 0;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
long SPIRamVFS::telldir(kvfs_dir_t* pdir) {
    const SPIRamDir* pDir = reinterpret_cast<SPIRamDir*>(pdir);

    if (!pDir) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }
    return reinterpret_cast<long>(pDir->offset);
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
void SPIRamVFS::seekdir(kvfs_dir_t* pdir, long offset) {
    SPIRamDir* pDir = reinterpret_cast<SPIRamDir*>(pdir);
    SPIRamNode* pNodeOffset = reinterpret_cast<SPIRamNode*>(offset);

    if (!pDir) return;
    // espidf calls seekdir(dir, 0) on rewinddir, since we use offset for other purpose, we've to
    // workarround
    if (!pNodeOffset) pDir->offset = pDir->pNode->pChild;
    else pDir->offset = pNodeOffset;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::mkdir(const char* path, mode_t mode) {
    if (!path) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }

    const char* lastSlash = strrchr(path, '/');
    const char* basename = lastSlash ? lastSlash + 1 : path;

    if (basename[0] == '\0') {
        errno = ENOENT;
        return ERR_BYTES_COUNT;
    }

    SPIRamNode* pParent;
    if (!lastSlash) {
        pParent = pRoot;
    } else {
        strcpy(dirBuffer, path);
        dirBuffer[lastSlash - path] = '\0';
        pParent = findNode(dirBuffer);
        if (!pParent) {
            errno = ENOENT;
            return ERR_BYTES_COUNT;
        }
    }

    const SPIRamNode* pNode = createNode(basename, VFS_SPIRAM_DIR_MODE_DEFAULT, pParent);
    if (!pNode) return ERR_BYTES_COUNT; // errno set by createNode

    KSVFS_DBG printf("[SPIRamVFS] mkdir '%s'\n", path);
    return 0;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::rmdir(const char* path) {
    if (!path) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }

    SPIRamNode* pNode = findNode(path);
    if (!pNode) {
        errno = ENOENT;
        return ERR_BYTES_COUNT;
    }
    if (!S_ISDIR(pNode->mode)) {
        errno = ENOTDIR;
        return ERR_BYTES_COUNT;
    }
    if (pNode == pRoot) {
        errno = EBUSY;
        return ERR_BYTES_COUNT;
    }
    if (pNode->pChild) {
        errno = ENOTEMPTY;
        return ERR_BYTES_COUNT;
    }
    if (pNode->refcount) {
        errno = EBUSY;
        return ERR_BYTES_COUNT;
    }

    deleteNode(pNode);
    KSVFS_DBG printf("[SPIRamVFS] rmdir '%s'\n", path);
    return 0;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::rename(const char* src, const char* dst) {
    if (!src || !dst) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }

    SPIRamNode* pNode = findNode(src);
    if (!pNode) {
        errno = ENOENT;
        return ERR_BYTES_COUNT;
    }
    if (pNode == pRoot) {
        errno = EBUSY;
        return ERR_BYTES_COUNT;
    }

    // find dst parent and basename
    const char* lastSlash = strrchr(dst, '/');
    const char* basename = lastSlash ? lastSlash + 1 : dst;

    if (basename[0] == '\0') {
        errno = ENOENT;
        return ERR_BYTES_COUNT;
    }

    SPIRamNode* pDstParent;
    if (!lastSlash) {
        pDstParent = pRoot;
    } else {
        strcpy(dirBuffer, dst);
        dirBuffer[lastSlash - dst] = '\0';
        pDstParent = findNode(dirBuffer);
        if (!pDstParent) {
            errno = ENOENT;
            return ERR_BYTES_COUNT;
        }
    }

    // check dst doesn't already exist
    SPIRamNode* pDstNode = findNode(dst);
    if (pDstNode) {
        // if dst exists and is a non-empty dir, fail
        if (S_ISDIR(pDstNode->mode) && pDstNode->pChild) {
            errno = ENOTEMPTY;
            return ERR_BYTES_COUNT;
        }
        // if types mismatch
        if (S_ISDIR(pNode->mode) != S_ISDIR(pDstNode->mode)) {
            errno = S_ISDIR(pNode->mode) ? ENOTDIR : EISDIR;
            return ERR_BYTES_COUNT;
        }
        deleteNode(pDstNode);
    }

    // detach from current parent
    SPIRamNode* pSrcParent = pNode->pParent;
    if (pSrcParent->pChild == pNode) {
        pSrcParent->pChild = pNode->pNext;
    } else {
        SPIRamNode* prev = pSrcParent->pChild;
        while (prev && prev->pNext != pNode)
            prev = prev->pNext;
        if (prev) prev->pNext = pNode->pNext;
    }

    // reattach to dst parent
    strncpy(pNode->name, basename, VFS_SPIRAM_NAME_MAX - 1);
    pNode->name[VFS_SPIRAM_NAME_MAX - 1] = '\0';
    pNode->nhash = spiram_vfs_hash(basename, pDstParent->nhash);
    pNode->pParent = pDstParent;
    pNode->pNext = pDstParent->pChild;
    pDstParent->pChild = pNode;
    pNode->mtime = pNode->ctime = time(NULL);

    KSVFS_DBG printf("[SPIRamVFS] rename '%s' -> '%s'\n", src, dst);
    return 0;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
SPIRamNode* SPIRamVFS::findNode(const char* path, SPIRamNode* pStart) {
    if (!path) return NULL;

    // Treat empty path or "/" as root
    if (path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
        return pRoot;
    }

    const char* cur = path;

    // skip the leading slashes
    while (*cur == '/')
        cur++;

    // pRoot is a fallback node here
    SPIRamNode* pNode = (pStart) ? pStart : pRoot;

    // walk the tree!
    while (*cur && pNode) {
        const char* endToken = strchr(cur, '/');
        size_t tokenLen = (endToken) ? (size_t)(endToken - cur) : strlen(cur);

        // This path shouldn't be allowed to exist
        if (tokenLen >= VFS_SPIRAM_NAME_MAX) return NULL;

        // Skipping double slash
        if (tokenLen == 0) {
            cur++;
            continue;
        }

        char* token = buffToken;
        // extract token
        memcpy(token, cur, tokenLen);
        token[tokenLen] = '\0';

        // handle current dir
        if (strcmp(token, ".") == 0) {
            cur = endToken ? endToken + 1 : cur + tokenLen;
            continue;
        }

        // handle parent dir
        if (strcmp(token, "..") == 0) {
            // NOTE: pRoot->pParent is always pRoot
            pNode = pNode->pParent;

            cur = endToken ? endToken + 1 : cur + tokenLen;
            continue;
        }

        // calculate hash for current path
        uint32_t curHash = spiram_vfs_hash(token, pNode->nhash);

        SPIRamNode* pChild = pNode->pChild;
        pNode = NULL;

        // walk down the path
        while (pChild) {
            bool found = pChild->nhash == curHash && (strcmp(pChild->name, token) == 0);

            if (found) {
                pNode = pChild;
                break;
            }
            pChild = pChild->pNext;
        }

        cur = endToken ? endToken + 1 : cur + tokenLen;

        // nothing found
        if (!endToken) break;
    }

    return pNode;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::access(const char* path, int amode) {
    if (!path) {
        errno = EINVAL;
        return ERR_BYTES_COUNT;
    }

    const SPIRamNode* pNode = findNode(path);
    if (!pNode) {
        errno = ENOENT;
        return ERR_BYTES_COUNT;
    }

    // F_OK — just check existence, already done above
    if (amode == F_OK) return 0;

    // extract owner permission bits (we have no users, treat as owner)
    mode_t m = pNode->mode;

    if ((amode & R_OK) && !(m & S_IRUSR)) {
        errno = EACCES;
        return ERR_BYTES_COUNT;
    }
    if ((amode & W_OK) && !(m & S_IWUSR)) {
        errno = EACCES;
        return ERR_BYTES_COUNT;
    }
    if ((amode & X_OK) && !(m & S_IXUSR)) {
        errno = EACCES;
        return ERR_BYTES_COUNT;
    }

    return 0;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
SPIRamNode* SPIRamVFS::createNode(const char* name, mode_t mode, SPIRamNode* pParent) {
    if (!pParent || !name || name[0] == '\0') {
        errno = EINVAL;
        return NULL;
    }

    if (!S_ISDIR(pParent->mode)) {
        errno = ENOTDIR;
        return NULL;
    }

    if (strlen(name) >= VFS_SPIRAM_NAME_MAX) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    uint32_t nHash = spiram_vfs_hash(name, pParent->nhash);

    // Check node existence
    SPIRamNode* scan = pParent->pChild;
    while (scan) {
        if (scan->nhash == nHash && strcmp(scan->name, name) == 0) {
            errno = EEXIST;
            return NULL;
        }
        scan = scan->pNext;
    }

    // Creating a new node
    SPIRamNode* pNewNode = new SPIRamNode();

    // Fill node with inital data
    strncpy(pNewNode->name, name, VFS_SPIRAM_NAME_MAX - 1);
    pNewNode->name[VFS_SPIRAM_NAME_MAX - 1] = '\0';
    pNewNode->nhash = nHash;
    pNewNode->mode = mode;
    pNewNode->pParent = pParent;
    pNewNode->atime = pNewNode->mtime = pNewNode->ctime = time(NULL);
    pNewNode->pNext = pParent->pChild;
    pParent->pChild = pNewNode;

    KSVFS_DBG printf("[SPIRamVFS] createNode '%s'\n", name);

    return pNewNode;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
void SPIRamVFS::fillNodeStat(const SPIRamNode* pNode, struct stat* st) {
    memset(st, 0, sizeof(*st));
    st->st_mode = pNode->mode;
    st->st_size = static_cast<off_t>(pNode->fsize);
    st->st_atime = pNode->atime;
    st->st_mtime = pNode->mtime;
    st->st_ctime = pNode->ctime;
    st->st_nlink = 1;
    st->st_blksize = VFS_SPIRAM_BLOCK_SIZE;
    st->st_blocks = (blkcnt_t)pNode->blocks.size();
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
void SPIRamVFS::deleteNode(SPIRamNode* pNode) {
    if (!pNode) return;

    SPIRamNode* parent = pNode->pParent;
    if (parent) {
        // Remove from sibling linked list
        if (parent->pChild == pNode) {
            parent->pChild = pNode->pNext;
        } else {
            SPIRamNode* prev = parent->pChild;
            while (prev && prev->pNext != pNode)
                prev = prev->pNext;
            if (prev) prev->pNext = pNode->pNext;
        }
    }

    // Free file blocks
    for (auto* block : pNode->blocks)
        heap_caps_free(block);

    delete pNode;
    KSVFS_DBG printf("[SPIRamVFS] deleteNode done\n");

    // TODO: maybe reattach pChild to parent -> next-> next->next ?
    // this might allow us to avoid recursion in a cleanup
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
int SPIRamVFS::allocfd(SPIRamNode* pNode, int flags) {
    for (int i = 0; i < VFS_SPIRAM_MAX_FD; ++i) {
        if (!pfds[i].pNode) {
            pfds[i].pNode = pNode;
            pfds[i].offset = 0;
            pfds[i].flags = flags;
            pNode->refcount++;
            KSVFS_DBG printf("[SPIRamVFS] allocfd -> %d (node='%s')\n", i, pNode->name);
            return i;
        }
    }
    errno = EMFILE;
    return ERR_BYTES_COUNT;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
void* SPIRamVFS::spiram_vfs_mem_alloc(size_t size) {
    uint32_t caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
    if (heap_caps_get_free_size(caps) > VFS_SPIRAM_RESERVED_SPACE) return heap_caps_malloc(size, caps);
    else return NULL;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
uint32_t SPIRamVFS::spiram_vfs_hash(const char* rpath, uint32_t parentHash) {
    uint32_t hash = parentHash;

    while (*rpath) {
        // skip begining slashes
        while (*rpath == '/')
            ++rpath;

        if (*rpath == '\0') break;

        const char* start = rpath;

        // find component end
        while (*rpath && *rpath != '/')
            ++rpath;

        size_t len = (size_t)(rpath - start);

        hash = VFS_SPIRAM_NAME_HASH(start, len, hash);

        // Append separator for both file and dir, cause, it doesn't actually matter
        hash = VFS_SPIRAM_NAME_HASH("/", 1, hash);
    }

    return hash;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
SPIRamVFS::SPIRamVFS(const char* mountPoint) : KeiraVFS(mountPoint) {
    // Zero memory associated with file descriptors
    memset(pfds, 0, sizeof(pfds));
    //--------------------------------------------------------------------------------- (^_^)==\~~

    // Create root node for a filesystem and setup it's params
    pRoot = new SPIRamNode();
    strcpy(pRoot->name, "/"); // not sure, maybe should be empty string instead
    pRoot->pParent = pRoot;
    pRoot->nhash = VFS_SPIRAM_HASH_INITAL;
    pRoot->mode = VFS_SPIRAM_DIR_MODE_DEFAULT;
    pRoot->atime = pRoot->mtime = pRoot->ctime = time(NULL);
    //--------------------------------------------------------------------------------- (^_^)==\~~
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
SPIRamVFS::~SPIRamVFS() {
    // Oh-boy, I can feel it...

    // Recursively walk all nodes, and, for each node we've to cleanup
    // file blocks

    // time to delete pRoot
    delete pRoot;
}
//-------------------------------------------------------------------------------------- (^_^)==\~

//================================================================================================
// cleaning local macro
//================================================================================================
#undef FD_ISVALID
#undef FD_ACCMODE
#undef FD_ISWRITABLE
//-------------------------------------------------------------------------------------- (^_^)==\~