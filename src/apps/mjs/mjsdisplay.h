#pragma once

#include "mjs.h"

class App;

/// Register the `display` and `colors` objects in the mJS global scope.
/// Also stores App* as __app__ global for use by other modules.
void mjs_display_register(struct mjs* mjs, App* app);
