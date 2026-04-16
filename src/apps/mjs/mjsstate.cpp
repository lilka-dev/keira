#include "mjsstate.h"
#include <lilka.h>
#include "mjs.h"

// state.save() - saves current state object to file
static void mjs_state_save(struct mjs* mjs) {
    mjs_val_t state_path_val = mjs_get(mjs, mjs_get_global(mjs), "__state_path__", ~0);
    if (!mjs_is_string(state_path_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    size_t path_len;
    const char* path = mjs_get_string(mjs, &state_path_val, &path_len);

    mjs_val_t state = mjs_get(mjs, mjs_get_global(mjs), "state", ~0);
    if (!mjs_is_object(state)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    FILE* file = fopen(path, "w");
    if (!file) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    // Iterate over state object properties
    // mJS provides mjs_next for object iteration
    mjs_val_t key_val, iter = mjs_mk_undefined();
    while ((key_val = mjs_next(mjs, state, &iter)) != mjs_mk_undefined()) {
        size_t key_len;
        const char* key = mjs_get_string(mjs, &key_val, &key_len);

        // Skip built-in methods
        if (strcmp(key, "save") == 0 || strcmp(key, "reset") == 0 || strcmp(key, "clear") == 0) {
            continue;
        }

        mjs_val_t val = mjs_get(mjs, state, key, ~0);

        if (mjs_is_number(val)) {
            fprintf(file, "%s\nnumber\n%lf\n", key, mjs_get_double(mjs, val));
        } else if (mjs_is_string(val)) {
            size_t vlen;
            const char* vstr = mjs_get_string(mjs, &val, &vlen);
            fprintf(file, "%s\nstring\n%s\n", key, vstr);
        } else if (mjs_is_boolean(val)) {
            fprintf(file, "%s\nboolean\n%d\n", key, mjs_get_bool(mjs, val));
        }
    }

    fclose(file);
    mjs_return(mjs, mjs_mk_undefined());
}

// state.reset() - reloads state from file
static void mjs_state_reset(struct mjs* mjs) {
    mjs_val_t state_path_val = mjs_get(mjs, mjs_get_global(mjs), "__state_path__", ~0);
    if (!mjs_is_string(state_path_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    size_t path_len;
    const char* path = mjs_get_string(mjs, &state_path_val, &path_len);

    mjs_state_load(mjs, path);
    mjs_return(mjs, mjs_mk_undefined());
}

// state.clear() - deletes state file and sets state to empty
static void mjs_state_clear(struct mjs* mjs) {
    mjs_val_t state_path_val = mjs_get(mjs, mjs_get_global(mjs), "__state_path__", ~0);
    if (!mjs_is_string(state_path_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    size_t path_len;
    const char* path = mjs_get_string(mjs, &state_path_val, &path_len);

    remove(path);

    // Create fresh state object with methods
    mjs_val_t state = mjs_mk_object(mjs);
    mjs_set(mjs, state, "save", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_state_save));
    mjs_set(mjs, state, "reset", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_state_reset));
    mjs_set(mjs, state, "clear", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_state_clear));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "state", ~0, state);

    mjs_return(mjs, mjs_mk_undefined());
}

void mjs_state_load(struct mjs* mjs, const char* path) {
    mjs_val_t state = mjs_mk_object(mjs);

    FILE* file = fopen(path, "r");
    if (file) {
        char key[256];
        char type[32];
        while (fscanf(file, "%255s", key) != EOF) {
            fscanf(file, "%31s", type);
            if (strcmp(type, "number") == 0) {
                double value;
                fscanf(file, "%lf", &value);
                mjs_set(mjs, state, key, ~0, mjs_mk_number(mjs, value));
            } else if (strcmp(type, "string") == 0) {
                char value[256];
                fgetc(file);
                fgets(value, 256, file);
                value[strcspn(value, "\n")] = '\0';
                mjs_set(mjs, state, key, ~0, mjs_mk_string(mjs, value, ~0, 1));
            } else if (strcmp(type, "boolean") == 0) {
                int value;
                fscanf(file, "%d", &value);
                mjs_set(mjs, state, key, ~0, mjs_mk_boolean(mjs, value));
            }
        }
        fclose(file);
    }

    // Add methods
    mjs_set(mjs, state, "save", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_state_save));
    mjs_set(mjs, state, "reset", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_state_reset));
    mjs_set(mjs, state, "clear", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_state_clear));

    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "state", ~0, state);
}

void mjs_state_register(struct mjs* mjs, const char* statePath) {
    // Store state path as global __state_path__
    mjs_val_t global = mjs_get_global(mjs);
    if (statePath) {
        mjs_set(mjs, global, "__state_path__", ~0, mjs_mk_string(mjs, statePath, ~0, 1));
        mjs_state_load(mjs, statePath);
    } else {
        // Create empty state object
        mjs_val_t state = mjs_mk_object(mjs);
        mjs_set(mjs, state, "save", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_state_save));
        mjs_set(mjs, state, "reset", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_state_reset));
        mjs_set(mjs, state, "clear", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_state_clear));
        mjs_set(mjs, global, "state", ~0, state);
    }
}
