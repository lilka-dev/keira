#pragma once

#include "mjs.h"

/// Register the `util` object in the mJS global scope.
/// Provides: util.time, util.sleep, util.exit, util.free_ram, util.total_ram.
///
void mjs_util_register(struct mjs* mjs);
