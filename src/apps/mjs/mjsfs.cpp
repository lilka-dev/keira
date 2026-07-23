#include "mjsfs.h"
#include <lilka.h>
#include "mjs.h"

////////////////////////////////////////////////////////
// Helper utils                                       //
////////////////////////////////////////////////////////
String mjspath_to_path(struct mjs* mjs, const char* path) {
    // TODO : speedify similar cases via per cpu ring buffers
    // Empty path
    if (strlen(path) == 0) return "/";

    // Absolute path
    if (path[0] == '/') return String(path);

    // Relative path
    mjs_val_t dir_val = mjs_get(mjs, mjs_get_global(mjs), "__dir__", ~0);

    const char* dir = mjs_get_cstring(mjs, &dir_val);

    // Extremely strange case
    if (!dir) return "/";

    return String(dir) + "/" + path;
}

// performs path traversal
// TODO: utilize same in resources
bool mjs_arg_to_path(struct mjs* mjs, int arg_index, String& out_path) {
    mjs_val_t arg = mjs_arg(mjs, arg_index);
    const char* arg_cstr = mjs_get_cstring(mjs, &arg);
    if (arg_cstr == NULL) return false;

    out_path = mjspath_to_path(mjs, arg_cstr);
    return true;
}

////////////////////////////////////////////////////////
// Common fs operations                               //
////////////////////////////////////////////////////////

// fs.ls(path) -> array of names
static void mjs_fs_ls(struct mjs* mjs) {
    String arg0_str;
    if (!mjs_arg_to_path(mjs, 0, arg0_str)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    const char* path = arg0_str.c_str();

    // Check if path is a cstring
    if (path == NULL) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    // Trying to open dir
    DIR* dir = opendir(path);

    if (!dir) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    mjs_val_t arr = mjs_mk_array(mjs);

    // Iterate over directory
    const struct dirent* dir_entry = NULL;
    while ((dir_entry = readdir(dir)) != NULL) {
        mjs_array_push(mjs, arr, mjs_mk_string(mjs, dir_entry->d_name, ~0, 1));
    }

    // Never forget
    closedir(dir);

    mjs_return(mjs, arr);
}

// fs.mkpath(path) -> bool
static void mjs_fs_mkpath(struct mjs* mjs) {
    String arg0_str;
    if (!mjs_arg_to_path(mjs, 0, arg0_str)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    const char* path = arg0_str.c_str();

    // TODO utilize pr#120
    char buf[PATH_MAX + 1];

    strncpy(buf, path, PATH_MAX);
    buf[PATH_MAX] = '\0';

    // Skip root "/<mount>"
    const char* p = strchr(buf + 1, '/');
    if (!p) {
        mjs_return(mjs, mjs_mk_boolean(mjs, false));
        return;
    }

    for (;;) {
        char* sep = strchr(p + 1, '/');
        if (sep) *sep = '\0';

        if (mkdir(buf, 0777) != 0 && errno != EEXIST) {
            mjs_return(mjs, mjs_mk_boolean(mjs, false));
            return;
        }

        if (!sep) break;

        *sep = '/';
        p = sep;
    }

    errno = 0;
    mjs_return(mjs, mjs_mk_boolean(mjs, true));
}

// fs.joinpath(path) -> str
static void mjs_fs_joinpath(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    const char* lpath = mjs_get_cstring(mjs, &arg0);
    if (lpath == NULL) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    mjs_val_t arg1 = mjs_arg(mjs, 1);
    const char* rpath = mjs_get_cstring(mjs, &arg1);
    if (rpath == NULL) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    String path = lilka::fileutils.joinPath(lpath, rpath);
    mjs_return(mjs, mjs_mk_string(mjs, path.c_str(), ~0, 1));
}

// fs.remove(path) -> bool
static void mjs_fs_remove(struct mjs* mjs) {
    String arg0_str;
    if (!mjs_arg_to_path(mjs, 0, arg0_str)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    const char* path = arg0_str.c_str();

    // Check if path is a cstring
    if (path == NULL) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    if (remove(path) == 0) {
        mjs_return(mjs, mjs_mk_boolean(mjs, true));
        return;
    }

    mjs_return(mjs, mjs_mk_boolean(mjs, false));
}

// fs.rename(oldName, newName) -> bool
static void mjs_fs_rename(struct mjs* mjs) {
    String arg0_str;
    if (!mjs_arg_to_path(mjs, 0, arg0_str)) {
        mjs_return(mjs, mjs_mk_boolean(mjs, false));
        return;
    }

    const char* oldName = arg0_str.c_str();

    String arg1_str;
    if (!mjs_arg_to_path(mjs, 1, arg1_str)) {
        mjs_return(mjs, mjs_mk_boolean(mjs, false));
        return;
    }

    const char* newName = arg1_str.c_str();

    if (rename(oldName, newName) == 0) {
        mjs_return(mjs, mjs_mk_boolean(mjs, true));
        return;
    };
    
    mjs_return(mjs, mjs_mk_boolean(mjs, false));
}

////////////////////////////////////////////////////////
// Opened file object operations                      //
////////////////////////////////////////////////////////

// file.size(file) -> number
static void mjs_fs_file_size(struct mjs* mjs) {
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
static void mjs_fs_file_seek(struct mjs* mjs) {
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
static void mjs_fs_file_read(struct mjs* mjs) {
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
static void mjs_fs_file_write(struct mjs* mjs) {
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
static void mjs_fs_file_exists(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    mjs_return(mjs, mjs_mk_boolean(mjs, mjs_is_foreign(ptr_val)));
}

// file.close(file)
static void mjs_fs_file_close(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (mjs_is_foreign(ptr_val)) {
        FILE* fp = static_cast<FILE*>(mjs_get_ptr(mjs, ptr_val));
        fclose(fp);
        mjs_set(mjs, obj, "pointer", ~0, mjs_mk_null());
    }
    mjs_return(mjs, mjs_mk_undefined());
}

// Internal file pointer storage (since mJS doesn't have userdata/metatables)
// We store the FILE* as a foreign pointer in the returned object
// sdcard.open(path[, mode]) -> {pointer, size(), seek(), read(), write(), exists(), close()}
static void mjs_fs_open(struct mjs* mjs) {
    String arg0_str;
    if (!mjs_arg_to_path(mjs, 0, arg0_str)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    const char* path = arg0_str.c_str();

    const char* mode = "r";
    mjs_val_t mode_arg = mjs_arg(mjs, 1);
    if (mjs_is_string(mode_arg)) {
        mode = mjs_get_cstring(mjs, &mode_arg);
    }

    FILE* fp = fopen(path, mode);

    mjs_val_t obj = mjs_mk_object(mjs);
    if (fp) {
        mjs_set(mjs, obj, "pointer", ~0, mjs_mk_foreign(mjs, fp));
    } else {
        mjs_set(mjs, obj, "pointer", ~0, mjs_mk_null());
    }
    mjs_set(mjs, obj, "size", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_file_size));
    mjs_set(mjs, obj, "seek", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_file_seek));
    mjs_set(mjs, obj, "read", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_file_read));
    mjs_set(mjs, obj, "write", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_file_write));
    mjs_set(mjs, obj, "exists", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_file_exists));
    mjs_set(mjs, obj, "close", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_file_close));

    mjs_return(mjs, obj);
}

////////////////////////////////////////////////////////
// fs module registration                             //
////////////////////////////////////////////////////////

void mjs_fs_register(struct mjs* mjs) {
    mjs_val_t fs = mjs_mk_object(mjs);
    mjs_set(mjs, fs, "ls", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_ls));
    mjs_set(mjs, fs, "mkpath", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_mkpath));
    mjs_set(mjs, fs, "joinpath", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_joinpath));
    mjs_set(mjs, fs, "remove", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_remove));
    mjs_set(mjs, fs, "rename", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_rename));
    mjs_set(mjs, fs, "open", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_fs_open));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "fs", ~0, fs);
}
