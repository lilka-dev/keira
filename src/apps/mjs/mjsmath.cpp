#include "mjsmath.h"
#include <Arduino.h>
#include <math.h>
#include <lilka.h>
#include "mjs.h"

static void mjs_math_random(struct mjs* mjs) {
    int n = mjs_nargs(mjs);
    if (n == 0) {
        mjs_return(mjs, mjs_mk_number(mjs, (float)rand() / RAND_MAX));
    } else if (n == 1) {
        int max = mjs_get_int(mjs, mjs_arg(mjs, 0));
        mjs_return(mjs, mjs_mk_number(mjs, random(max)));
    } else {
        int min = mjs_get_int(mjs, mjs_arg(mjs, 0));
        int max = mjs_get_int(mjs, mjs_arg(mjs, 1));
        mjs_return(mjs, mjs_mk_number(mjs, random(min, max)));
    }
}

static void mjs_math_clamp(struct mjs* mjs) {
    double value = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double min = mjs_get_double(mjs, mjs_arg(mjs, 1));
    double max = mjs_get_double(mjs, mjs_arg(mjs, 2));
    mjs_return(mjs, mjs_mk_number(mjs, value < min ? min : (value > max ? max : value)));
}

static void mjs_math_lerp(struct mjs* mjs) {
    double a = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double b = mjs_get_double(mjs, mjs_arg(mjs, 1));
    double t = mjs_get_double(mjs, mjs_arg(mjs, 2));
    mjs_return(mjs, mjs_mk_number(mjs, a + (b - a) * t));
}

static void mjs_math_map(struct mjs* mjs) {
    double value = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double in_min = mjs_get_double(mjs, mjs_arg(mjs, 1));
    double in_max = mjs_get_double(mjs, mjs_arg(mjs, 2));
    double out_min = mjs_get_double(mjs, mjs_arg(mjs, 3));
    double out_max = mjs_get_double(mjs, mjs_arg(mjs, 4));
    mjs_return(mjs, mjs_mk_number(mjs, (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min));
}

static void mjs_math_abs(struct mjs* mjs) {
    double value = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, value < 0 ? -value : value));
}

static void mjs_math_sign(struct mjs* mjs) {
    double value = mjs_get_double(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, value > 0 ? 1 : (value < 0 ? -1 : 0)));
}

static void mjs_math_sqrt(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, sqrt(mjs_get_double(mjs, mjs_arg(mjs, 0)))));
}

static void mjs_math_pow(struct mjs* mjs) {
    mjs_return(
        mjs, mjs_mk_number(mjs, pow(mjs_get_double(mjs, mjs_arg(mjs, 0)), mjs_get_double(mjs, mjs_arg(mjs, 1))))
    );
}

static void mjs_math_min(struct mjs* mjs) {
    mjs_val_t arr = mjs_arg(mjs, 0);
    unsigned long len = mjs_array_length(mjs, arr);
    if (len == 0) return;
    double min = mjs_get_double(mjs, mjs_array_get(mjs, arr, 0));
    for (unsigned long i = 1; i < len; i++) {
        double val = mjs_get_double(mjs, mjs_array_get(mjs, arr, i));
        if (val < min) min = val;
    }
    mjs_return(mjs, mjs_mk_number(mjs, min));
}

static void mjs_math_max(struct mjs* mjs) {
    mjs_val_t arr = mjs_arg(mjs, 0);
    unsigned long len = mjs_array_length(mjs, arr);
    if (len == 0) return;
    double max = mjs_get_double(mjs, mjs_array_get(mjs, arr, 0));
    for (unsigned long i = 1; i < len; i++) {
        double val = mjs_get_double(mjs, mjs_array_get(mjs, arr, i));
        if (val > max) max = val;
    }
    mjs_return(mjs, mjs_mk_number(mjs, max));
}

static void mjs_math_sum(struct mjs* mjs) {
    mjs_val_t arr = mjs_arg(mjs, 0);
    unsigned long len = mjs_array_length(mjs, arr);
    double sum = 0;
    for (unsigned long i = 0; i < len; i++) {
        sum += mjs_get_double(mjs, mjs_array_get(mjs, arr, i));
    }
    mjs_return(mjs, mjs_mk_number(mjs, sum));
}

static void mjs_math_avg(struct mjs* mjs) {
    mjs_val_t arr = mjs_arg(mjs, 0);
    unsigned long len = mjs_array_length(mjs, arr);
    if (len == 0) return;
    double sum = 0;
    for (unsigned long i = 0; i < len; i++) {
        sum += mjs_get_double(mjs, mjs_array_get(mjs, arr, i));
    }
    mjs_return(mjs, mjs_mk_number(mjs, sum / len));
}

static void mjs_math_floor(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, floor(mjs_get_double(mjs, mjs_arg(mjs, 0)))));
}

static void mjs_math_ceil(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, ceil(mjs_get_double(mjs, mjs_arg(mjs, 0)))));
}

static void mjs_math_round(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, roundf(mjs_get_double(mjs, mjs_arg(mjs, 0)))));
}

static void mjs_math_sin(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, sin(mjs_get_double(mjs, mjs_arg(mjs, 0)))));
}

static void mjs_math_cos(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, cos(mjs_get_double(mjs, mjs_arg(mjs, 0)))));
}

static void mjs_math_tan(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, tan(mjs_get_double(mjs, mjs_arg(mjs, 0)))));
}

static void mjs_math_asin(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, asin(mjs_get_double(mjs, mjs_arg(mjs, 0)))));
}

static void mjs_math_acos(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, acos(mjs_get_double(mjs, mjs_arg(mjs, 0)))));
}

static void mjs_math_atan(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, atan(mjs_get_double(mjs, mjs_arg(mjs, 0)))));
}

static void mjs_math_atan2(struct mjs* mjs) {
    mjs_return(
        mjs, mjs_mk_number(mjs, atan2(mjs_get_double(mjs, mjs_arg(mjs, 0)), mjs_get_double(mjs, mjs_arg(mjs, 1))))
    );
}

static void mjs_math_log(struct mjs* mjs) {
    int n = mjs_nargs(mjs);
    double value = mjs_get_double(mjs, mjs_arg(mjs, 0));
    if (n == 1) {
        mjs_return(mjs, mjs_mk_number(mjs, log(value)));
    } else {
        double base = mjs_get_double(mjs, mjs_arg(mjs, 1));
        mjs_return(mjs, mjs_mk_number(mjs, log(value) / log(base)));
    }
}

static void mjs_math_deg(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, mjs_get_double(mjs, mjs_arg(mjs, 0)) * 180 / M_PI));
}

static void mjs_math_rad(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, mjs_get_double(mjs, mjs_arg(mjs, 0)) * M_PI / 180));
}

// Returns [nx, ny] array since mJS can't return multiple values
static void mjs_math_norm(struct mjs* mjs) {
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double y = mjs_get_double(mjs, mjs_arg(mjs, 1));
    double length = sqrt(x * x + y * y);
    mjs_val_t result = mjs_mk_array(mjs);
    mjs_array_push(mjs, result, mjs_mk_number(mjs, x / length));
    mjs_array_push(mjs, result, mjs_mk_number(mjs, y / length));
    mjs_return(mjs, result);
}

static void mjs_math_len(struct mjs* mjs) {
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double y = mjs_get_double(mjs, mjs_arg(mjs, 1));
    mjs_return(mjs, mjs_mk_number(mjs, sqrt(x * x + y * y)));
}

static void mjs_math_dist(struct mjs* mjs) {
    double x1 = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double y1 = mjs_get_double(mjs, mjs_arg(mjs, 1));
    double x2 = mjs_get_double(mjs, mjs_arg(mjs, 2));
    double y2 = mjs_get_double(mjs, mjs_arg(mjs, 3));
    mjs_return(mjs, mjs_mk_number(mjs, sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))));
}

// Returns [rx, ry] array (angle in degrees, Y-down, clockwise)
static void mjs_math_rotate(struct mjs* mjs) {
    double x = mjs_get_double(mjs, mjs_arg(mjs, 0));
    double y = mjs_get_double(mjs, mjs_arg(mjs, 1));
    double angle = mjs_get_double(mjs, mjs_arg(mjs, 2));
    float c = lilka::fCos360(angle);
    float s = lilka::fSin360(angle);
    mjs_val_t result = mjs_mk_array(mjs);
    mjs_array_push(mjs, result, mjs_mk_number(mjs, x * c - y * s));
    mjs_array_push(mjs, result, mjs_mk_number(mjs, x * s + y * c));
    mjs_return(mjs, result);
}

void mjs_math_register(struct mjs* mjs) {
    mjs_val_t math_obj = mjs_mk_object(mjs);

    mjs_set(mjs, math_obj, "random", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_random));
    mjs_set(mjs, math_obj, "clamp", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_clamp));
    mjs_set(mjs, math_obj, "lerp", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_lerp));
    mjs_set(mjs, math_obj, "map", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_map));
    mjs_set(mjs, math_obj, "abs", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_abs));
    mjs_set(mjs, math_obj, "sign", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_sign));
    mjs_set(mjs, math_obj, "sqrt", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_sqrt));
    mjs_set(mjs, math_obj, "pow", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_pow));
    mjs_set(mjs, math_obj, "min", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_min));
    mjs_set(mjs, math_obj, "max", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_max));
    mjs_set(mjs, math_obj, "sum", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_sum));
    mjs_set(mjs, math_obj, "avg", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_avg));
    mjs_set(mjs, math_obj, "floor", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_floor));
    mjs_set(mjs, math_obj, "ceil", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_ceil));
    mjs_set(mjs, math_obj, "round", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_round));
    mjs_set(mjs, math_obj, "sin", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_sin));
    mjs_set(mjs, math_obj, "cos", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_cos));
    mjs_set(mjs, math_obj, "tan", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_tan));
    mjs_set(mjs, math_obj, "asin", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_asin));
    mjs_set(mjs, math_obj, "acos", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_acos));
    mjs_set(mjs, math_obj, "atan", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_atan));
    mjs_set(mjs, math_obj, "atan2", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_atan2));
    mjs_set(mjs, math_obj, "log", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_log));
    mjs_set(mjs, math_obj, "deg", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_deg));
    mjs_set(mjs, math_obj, "rad", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_rad));
    mjs_set(mjs, math_obj, "norm", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_norm));
    mjs_set(mjs, math_obj, "len", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_len));
    mjs_set(mjs, math_obj, "dist", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_dist));
    mjs_set(mjs, math_obj, "rotate", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_math_rotate));

    // Constants
    mjs_set(mjs, math_obj, "pi", ~0, mjs_mk_number(mjs, M_PI));
    mjs_set(mjs, math_obj, "e", ~0, mjs_mk_number(mjs, M_E));
    mjs_set(mjs, math_obj, "tau", ~0, mjs_mk_number(mjs, M_PI * 2));

    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "math", ~0, math_obj);
}
