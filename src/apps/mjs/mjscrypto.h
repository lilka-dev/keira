#pragma once

#include "mjs.h"

/// Register the `crypto` object in the mJS global scope.
void mjs_crypto_register(struct mjs* mjs);
