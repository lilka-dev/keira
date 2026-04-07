#include "mjsgeometry.h"
#include "mjs.h"

// geometry.intersect_lines(ax, ay, bx, by, cx, cy, dx, dy) -> boolean
static void mjs_geometry_intersect_lines(struct mjs* mjs) {
    float ax = mjs_get_double(mjs, mjs_arg(mjs, 0));
    float ay = mjs_get_double(mjs, mjs_arg(mjs, 1));
    float bx = mjs_get_double(mjs, mjs_arg(mjs, 2));
    float by = mjs_get_double(mjs, mjs_arg(mjs, 3));
    float cx = mjs_get_double(mjs, mjs_arg(mjs, 4));
    float cy = mjs_get_double(mjs, mjs_arg(mjs, 5));
    float dx = mjs_get_double(mjs, mjs_arg(mjs, 6));
    float dy = mjs_get_double(mjs, mjs_arg(mjs, 7));

    float denominator = (by - ay) * (dx - cx) - (bx - ax) * (dy - cy);
    if (denominator == 0) {
        mjs_return(mjs, mjs_mk_boolean(mjs, 0));
        return;
    }

    float t = ((cy - ay) * (dx - cx) - (cx - ax) * (dy - cy)) / denominator;
    float u = -((ay - by) * (cx - ax) - (ax - bx) * (cy - ay)) / denominator;

    mjs_return(mjs, mjs_mk_boolean(mjs, t >= 0 && t <= 1 && u >= 0 && u <= 1));
}

// geometry.intersect_aabb(ax, ay, aw, ah, bx, by, bw, bh) -> boolean
static void mjs_geometry_intersect_aabb(struct mjs* mjs) {
    float ax = mjs_get_double(mjs, mjs_arg(mjs, 0));
    float ay = mjs_get_double(mjs, mjs_arg(mjs, 1));
    float aw = mjs_get_double(mjs, mjs_arg(mjs, 2));
    float ah = mjs_get_double(mjs, mjs_arg(mjs, 3));
    float bx = mjs_get_double(mjs, mjs_arg(mjs, 4));
    float by = mjs_get_double(mjs, mjs_arg(mjs, 5));
    float bw = mjs_get_double(mjs, mjs_arg(mjs, 6));
    float bh = mjs_get_double(mjs, mjs_arg(mjs, 7));

    mjs_return(mjs, mjs_mk_boolean(mjs, ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by));
}

void mjs_geometry_register(struct mjs* mjs) {
    mjs_val_t geometry = mjs_mk_object(mjs);
    mjs_set(
        mjs, geometry, "intersect_lines", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_geometry_intersect_lines)
    );
    mjs_set(mjs, geometry, "intersect_aabb", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_geometry_intersect_aabb));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "geometry", ~0, geometry);
}
