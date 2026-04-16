#include "mjssdcard.h"
#include <lilka.h>
#include "mjs.h"

// sdcard.ls(path) -> array of names
static void mjs_sdcard_ls(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    const char* path = mjs_get_cstring(mjs, &arg0);

    if (!lilka::fileutils.isSDAvailable()) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    size_t numEntries = lilka::fileutils.getEntryCount(&SD, path);
    if (numEntries == 0) {
        mjs_return(mjs, mjs_mk_array(mjs));
        return;
    }

    lilka::Entry* entries = new lilka::Entry[numEntries];
    int count = lilka::fileutils.listDir(&SD, path, entries);

    mjs_val_t arr = mjs_mk_array(mjs);
    for (int i = 0; i < count; i++) {
        mjs_array_push(mjs, arr, mjs_mk_string(mjs, entries[i].name.c_str(), ~0, 1));
    }
    delete[] entries;
    mjs_return(mjs, arr);
}

// sdcard.remove(path)
static void mjs_sdcard_remove(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    const char* path = mjs_get_cstring(mjs, &arg0);

    if (!lilka::fileutils.isSDAvailable()) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    remove((lilka::fileutils.getSDRoot() + String(path)).c_str());
    mjs_return(mjs, mjs_mk_undefined());
}

// sdcard.rename(oldName, newName)
static void mjs_sdcard_rename(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    mjs_val_t arg1 = mjs_arg(mjs, 1);
    const char* oldName = mjs_get_cstring(mjs, &arg0);
    const char* newName = mjs_get_cstring(mjs, &arg1);

    if (!lilka::fileutils.isSDAvailable()) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    rename(
        (lilka::fileutils.getSDRoot() + String(oldName)).c_str(),
        (lilka::fileutils.getSDRoot() + String(newName)).c_str()
    );
    mjs_return(mjs, mjs_mk_undefined());
}

// Internal file pointer storage (since mJS doesn't have userdata/metatables)
// We store the FILE* as a foreign pointer in the returned object
// sdcard.open(path[, mode]) -> {pointer, size(), seek(), read(), write(), exists(), close()}
static void mjs_sdcard_file_size(struct mjs* mjs);
static void mjs_sdcard_file_seek(struct mjs* mjs);
static void mjs_sdcard_file_read(struct mjs* mjs);
static void mjs_sdcard_file_write(struct mjs* mjs);
static void mjs_sdcard_file_exists(struct mjs* mjs);
static void mjs_sdcard_file_close(struct mjs* mjs);

static void mjs_sdcard_open(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    const char* path = mjs_get_cstring(mjs, &arg0);
    const char* mode = "r";
    mjs_val_t mode_arg = mjs_arg(mjs, 1);
    if (mjs_is_string(mode_arg)) {
        mode = mjs_get_cstring(mjs, &mode_arg);
    }

    FILE* fp = fopen((lilka::fileutils.getSDRoot() + String(path)).c_str(), mode);

    mjs_val_t obj = mjs_mk_object(mjs);
    if (fp) {
        mjs_set(mjs, obj, "pointer", ~0, mjs_mk_foreign(mjs, fp));
    } else {
        mjs_set(mjs, obj, "pointer", ~0, mjs_mk_null());
    }
    mjs_set(mjs, obj, "size", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_sdcard_file_size));
    mjs_set(mjs, obj, "seek", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_sdcard_file_seek));
    mjs_set(mjs, obj, "read", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_sdcard_file_read));
    mjs_set(mjs, obj, "write", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_sdcard_file_write));
    mjs_set(mjs, obj, "exists", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_sdcard_file_exists));
    mjs_set(mjs, obj, "close", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_sdcard_file_close));

    mjs_return(mjs, obj);
}

// file.size(file) -> number
static void mjs_sdcard_file_size(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_number(mjs, 0));
        return;
    }
    FILE* fp = static_cast<FILE*>(mjs_get_ptr(mjs, ptr_val));
    size_t cur = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, cur, SEEK_SET);
    mjs_return(mjs, mjs_mk_number(mjs, size));
}

// file.seek(file, offset)
static void mjs_sdcard_file_seek(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    FILE* fp = static_cast<FILE*>(mjs_get_ptr(mjs, ptr_val));
    size_t offset = mjs_get_int(mjs, mjs_arg(mjs, 1));
    fseek(fp, offset, SEEK_SET);
    mjs_return(mjs, mjs_mk_undefined());
}

// file.read(file, maxBytes) -> string
static void mjs_sdcard_file_read(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    FILE* fp = static_cast<FILE*>(mjs_get_ptr(mjs, ptr_val));
    size_t maxBytes = mjs_get_int(mjs, mjs_arg(mjs, 1));

    char* buffer = new char[maxBytes + 1];
    size_t bytesRead = fread(buffer, 1, maxBytes, fp);
    buffer[bytesRead] = 0;
    mjs_return(mjs, mjs_mk_string(mjs, buffer, bytesRead, 1));
    delete[] buffer;
}

// file.write(file, text)
static void mjs_sdcard_file_write(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    FILE* fp = static_cast<FILE*>(mjs_get_ptr(mjs, ptr_val));
    mjs_val_t arg1 = mjs_arg(mjs, 1);
    const char* text = mjs_get_cstring(mjs, &arg1);
    fprintf(fp, "%s", text);
    mjs_return(mjs, mjs_mk_undefined());
}

// file.exists(file) -> boolean
static void mjs_sdcard_file_exists(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    mjs_return(mjs, mjs_mk_boolean(mjs, mjs_is_foreign(ptr_val)));
}

// file.close(file)
static void mjs_sdcard_file_close(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (mjs_is_foreign(ptr_val)) {
        FILE* fp = static_cast<FILE*>(mjs_get_ptr(mjs, ptr_val));
        fclose(fp);
        mjs_set(mjs, obj, "pointer", ~0, mjs_mk_null());
    }
    mjs_return(mjs, mjs_mk_undefined());
}

void mjs_sdcard_register(struct mjs* mjs) {
    mjs_val_t sdcard = mjs_mk_object(mjs);
    mjs_set(mjs, sdcard, "ls", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_sdcard_ls));
    mjs_set(mjs, sdcard, "remove", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_sdcard_remove));
    mjs_set(mjs, sdcard, "rename", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_sdcard_rename));
    mjs_set(mjs, sdcard, "open", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_sdcard_open));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "sdcard", ~0, sdcard);
}
