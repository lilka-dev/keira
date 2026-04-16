#pragma once

#include "mjs.h"

/// Register the `console` object in the mJS global scope.
/// Provides: console.print(...).
///
void mjs_console_register(struct mjs* mjs);
