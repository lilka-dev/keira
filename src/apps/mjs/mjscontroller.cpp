#include "mjscontroller.h"
#include <lilka.h>
#include "mjs.h"

// controller.get_state() -> object with up/down/left/right/a/b/c/d/select/start
// Each key has {pressed, just_pressed, just_released}
static void mjs_controller_get_state(struct mjs* mjs) {
    lilka::State state = lilka::controller.getState();
    lilka::_StateButtons& buttons = *reinterpret_cast<lilka::_StateButtons*>(&state);

    const char* keys[] = {"up", "down", "left", "right", "a", "b", "c", "d", "select", "start"};

    mjs_val_t result = mjs_mk_object(mjs);
    for (int i = 0; i < 10; i++) {
        mjs_val_t btn = mjs_mk_object(mjs);
        mjs_set(mjs, btn, "pressed", ~0, mjs_mk_boolean(mjs, buttons[i].pressed));
        mjs_set(mjs, btn, "just_pressed", ~0, mjs_mk_boolean(mjs, buttons[i].justPressed));
        mjs_set(mjs, btn, "just_released", ~0, mjs_mk_boolean(mjs, buttons[i].justReleased));
        mjs_set(mjs, result, keys[i], ~0, btn);
    }

    mjs_return(mjs, result);
}

void mjs_controller_register(struct mjs* mjs) {
    mjs_val_t controller = mjs_mk_object(mjs);
    mjs_set(mjs, controller, "get_state", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_controller_get_state));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "controller", ~0, controller);
}
