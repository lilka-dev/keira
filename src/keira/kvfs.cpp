#include "kvfs.h"

KeiraVFS::KeiraVFS(const char* path) {
    strcpy(this->rootDir, path);
}

void KeiraVFS::reg() {
    esp_vfs_t vfs = {};
    // let's hope this hack works :D
    esp_vfs_t v = {};
    v.flags = ESP_VFS_FLAG_CONTEXT_PTR; // Use the context-pointer (_p) versions

    // our vtable exactly fits the esp_vfs_t description, so we use this
    // elegant way and just copy it in esp_vfs_t without mindfuck with a c++ness
    // this makes wrapper makings for almost anything in esp_idf incredibly simple
    // and incredibly fast as well
    void** vtable = *(void***)this;
    void** vfs_table = (void**)((uintptr_t)&v + sizeof(v.flags));
    memcpy(vfs_table, vtable, sizeof(esp_vfs_t) - sizeof(v.flags));

    // register
    if (esp_vfs_register(rootDir, &v, this) == ESP_OK) {
        this->registered = true;
    }
}

void KeiraVFS::unreg() {
    if (this->registered) esp_vfs_unregister(rootDir);
    registered = false;
}
