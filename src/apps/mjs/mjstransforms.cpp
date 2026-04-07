#include "mjstransforms.h"
#include <lilka.h>
#include "mjs.h"

// transforms.new() -> transform object {pointer}
static void mjs_transforms_new(struct mjs* mjs) {
    lilka::Transform* t = new lilka::Transform();
    mjs_val_t obj = mjs_mk_object(mjs);
    mjs_set(mjs, obj, "pointer", ~0, mjs_mk_foreign(mjs, t));
    mjs_return(mjs, obj);
}

// transform.rotate(transform, angle) -> new transform object
static void mjs_transforms_rotate(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Transform* t = static_cast<lilka::Transform*>(mjs_get_ptr(mjs, ptr_val));
    float angle = mjs_get_double(mjs, mjs_arg(mjs, 1));

    lilka::Transform* result = new lilka::Transform(t->rotate(angle));
    mjs_val_t result_obj = mjs_mk_object(mjs);
    mjs_set(mjs, result_obj, "pointer", ~0, mjs_mk_foreign(mjs, result));
    mjs_return(mjs, result_obj);
}

// transform.scale(transform, sx, sy) -> new transform object
static void mjs_transforms_scale(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Transform* t = static_cast<lilka::Transform*>(mjs_get_ptr(mjs, ptr_val));
    float sx = mjs_get_double(mjs, mjs_arg(mjs, 1));
    float sy = mjs_get_double(mjs, mjs_arg(mjs, 2));

    lilka::Transform* result = new lilka::Transform(t->scale(sx, sy));
    mjs_val_t result_obj = mjs_mk_object(mjs);
    mjs_set(mjs, result_obj, "pointer", ~0, mjs_mk_foreign(mjs, result));
    mjs_return(mjs, result_obj);
}

// transform.multiply(transform, other) -> new transform object
static void mjs_transforms_multiply(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t other = mjs_arg(mjs, 1);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    mjs_val_t ptr_val2 = mjs_get(mjs, other, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val) || !mjs_is_foreign(ptr_val2)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Transform* t1 = static_cast<lilka::Transform*>(mjs_get_ptr(mjs, ptr_val));
    lilka::Transform* t2 = static_cast<lilka::Transform*>(mjs_get_ptr(mjs, ptr_val2));

    lilka::Transform* result = new lilka::Transform(t1->multiply(*t2));
    mjs_val_t result_obj = mjs_mk_object(mjs);
    mjs_set(mjs, result_obj, "pointer", ~0, mjs_mk_foreign(mjs, result));
    mjs_return(mjs, result_obj);
}

// transform.inverse(transform) -> new transform object
static void mjs_transforms_inverse(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Transform* t = static_cast<lilka::Transform*>(mjs_get_ptr(mjs, ptr_val));

    lilka::Transform* result = new lilka::Transform(t->inverse());
    mjs_val_t result_obj = mjs_mk_object(mjs);
    mjs_set(mjs, result_obj, "pointer", ~0, mjs_mk_foreign(mjs, result));
    mjs_return(mjs, result_obj);
}

// transform.vtransform(transform, x, y) -> [x, y]
static void mjs_transforms_vtransform(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Transform* t = static_cast<lilka::Transform*>(mjs_get_ptr(mjs, ptr_val));
    int x = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int y = mjs_get_int(mjs, mjs_arg(mjs, 2));

    lilka::int_vector_t result = t->transform(lilka::int_vector_t{.x = x, .y = y});

    mjs_val_t arr = mjs_mk_array(mjs);
    mjs_array_push(mjs, arr, mjs_mk_number(mjs, result.x));
    mjs_array_push(mjs, arr, mjs_mk_number(mjs, result.y));
    mjs_return(mjs, arr);
}

// transform.get(transform) -> [[a, b], [c, d]]
static void mjs_transforms_get(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Transform* t = static_cast<lilka::Transform*>(mjs_get_ptr(mjs, ptr_val));

    mjs_val_t matrix = mjs_mk_array(mjs);
    for (int i = 0; i < 2; i++) {
        mjs_val_t row = mjs_mk_array(mjs);
        for (int j = 0; j < 2; j++) {
            mjs_array_push(mjs, row, mjs_mk_number(mjs, t->matrix[i][j]));
        }
        mjs_array_push(mjs, matrix, row);
    }
    mjs_return(mjs, matrix);
}

// transform.set(transform, matrix) - matrix is [[a, b], [c, d]]
static void mjs_transforms_set(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Transform* t = static_cast<lilka::Transform*>(mjs_get_ptr(mjs, ptr_val));
    mjs_val_t matrix = mjs_arg(mjs, 1);

    for (int i = 0; i < 2; i++) {
        mjs_val_t row = mjs_array_get(mjs, matrix, i);
        for (int j = 0; j < 2; j++) {
            t->matrix[i][j] = mjs_get_double(mjs, mjs_array_get(mjs, row, j));
        }
    }
    mjs_return(mjs, mjs_mk_undefined());
}

// transform.delete(transform) - free the transform
static void mjs_transforms_delete(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (mjs_is_foreign(ptr_val)) {
        delete static_cast<lilka::Transform*>(mjs_get_ptr(mjs, ptr_val));
        mjs_set(mjs, obj, "pointer", ~0, mjs_mk_null());
    }
    mjs_return(mjs, mjs_mk_undefined());
}

void mjs_transforms_register(struct mjs* mjs) {
    mjs_val_t transforms = mjs_mk_object(mjs);
    mjs_set(mjs, transforms, "create", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_transforms_new));
    mjs_set(mjs, transforms, "rotate", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_transforms_rotate));
    mjs_set(mjs, transforms, "scale", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_transforms_scale));
    mjs_set(mjs, transforms, "multiply", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_transforms_multiply));
    mjs_set(mjs, transforms, "inverse", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_transforms_inverse));
    mjs_set(mjs, transforms, "vtransform", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_transforms_vtransform));
    mjs_set(mjs, transforms, "get", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_transforms_get));
    mjs_set(mjs, transforms, "set", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_transforms_set));
    mjs_set(mjs, transforms, "free", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_transforms_delete));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "transforms", ~0, transforms);
}
