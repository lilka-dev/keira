#include "mjsconsole.h"
#include <Arduino.h>
#include "mjs.h"

// console.print(...) - prints values tab-separated with newline at the end
static void mjs_console_print(struct mjs* mjs) {
    int n = mjs_nargs(mjs);
    for (int i = 0; i < n; i++) {
        mjs_val_t arg = mjs_arg(mjs, i);
        if (mjs_is_string(arg)) {
            size_t len;
            const char* s = mjs_get_string(mjs, &arg, &len);
            printf("%.*s", (int)len, s);
        } else if (mjs_is_number(arg)) {
            printf("%g", mjs_get_double(mjs, arg));
        } else if (mjs_is_boolean(arg)) {
            printf("%s", mjs_get_bool(mjs, arg) ? "true" : "false");
        } else if (mjs_is_null(arg)) {
            printf("null");
        } else if (mjs_is_undefined(arg)) {
            printf("undefined");
        } else if (mjs_is_foreign(arg)) {
            printf("[foreign %p]", mjs_get_ptr(mjs, arg));
        } else if (mjs_is_object(arg)) {
            printf("[object]");
        } else if (mjs_is_array(arg)) {
            printf("[array]");
        } else {
            printf("[unknown]");
        }
        if (i < n - 1) {
            printf("\t");
        }
    }
    printf("\n");
}

void mjs_console_register(struct mjs* mjs) {
    mjs_val_t console = mjs_mk_object(mjs);

    mjs_set(mjs, console, "print", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_console_print));

    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "console", ~0, console);
}
