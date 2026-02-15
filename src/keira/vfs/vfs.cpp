#include "vfs.h"

KeiraVFS::KeiraVFS(const char* path) {
    strcpy(this->rootDir, path);
}

// Returns current offset in a directory stream
// @param pdir pointer to directory stream
// @returns offset
long KeiraVFS::telldir(kvfs_dir_t* pdir) {
    KVFS_DBG lilka::serial.log("[KVFS] %s offset %d\n", __PRETTY_FUNCTION__, pdir->offset);
    return pdir->offset;
}

// Sets current directory stream offset. Note offset should be a number
// once provided by telldir only, cause doesn't always meant to be an
// index
// @param pdir pointer to directory stream
void KeiraVFS::seekdir(kvfs_dir_t* pdir, long offset) {
    KVFS_DBG lilka::serial.log("[KVFS] %s to new offset:%d\n", __PRETTY_FUNCTION__, offset);
    pdir->offset = offset;
}

void KeiraVFS::reg() {
    esp_vfs_t v = {};

    // Use the context-pointer (_p) versions
    v.flags = ESP_VFS_FLAG_CONTEXT_PTR;

    // our vtable exactly fits the esp_vfs_t description, so we use this
    // elegant way and just copy it in esp_vfs_t without mindfuck with a c++ness
    // this makes wrapper makings for almost anything in esp_idf incredibly simple
    // and incredibly fast as well
    void** vtable = *(void***)this;
    void** vfs_table = (void**)((uintptr_t)&v + sizeof(v.flags));
    memcpy(vfs_table, vtable, sizeof(esp_vfs_t) - sizeof(v.flags));

    // register
    if (!registered && (esp_vfs_register(rootDir, &v, this) == ESP_OK)) {
        this->registered = true;
    }
}

void KeiraVFS::unreg() {
    if (this->registered) esp_vfs_unregister(rootDir);
    registered = false;
}
