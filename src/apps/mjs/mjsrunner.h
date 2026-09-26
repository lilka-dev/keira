#pragma once

#include <lilka.h>
#include "keira/app.h"

struct mjs;

// load() replacement that resolves relative paths against __dir__ global
void mjs_custom_load(struct mjs* mjs);

class MJSApp : public App {
public:
    explicit MJSApp(String path);
    void run() override;
    static void* ffi_resolver(void* handle, const char* name);

private:
    String path;
};
