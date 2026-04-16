#pragma once

#include "mjs.h"

/// Register the `state` object in the mJS global scope.
/// @param statePath Path to the .state file (or NULL if not running from file).
void mjs_state_register(struct mjs* mjs, const char* statePath);

/// Load state from file into the global `state` object.
void mjs_state_load(struct mjs* mjs, const char* path);
