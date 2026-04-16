#pragma once

#include "mjs.h"

/// Register the `resources` object in the mJS global scope.
/// @param dir The directory of the script being executed (for resolving relative paths).
void mjs_resources_register(struct mjs* mjs, const char* dir);
