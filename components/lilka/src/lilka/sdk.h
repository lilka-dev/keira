#pragma once

#include <lilka.h>
#include "version_auto_gen.h"

// TODO: Write Docs for lilka/sdk.h

namespace lilka {

typedef enum SDKVersionType : uint8_t {
    SDK_VERSION_TYPE_DEV = 0,
    SDK_VERSION_TYPE_PRE_RELEASE = 1,
    SDK_VERSION_TYPE_RELEASE = 2
} version_type_t;

extern char SDK_VERSION_TYPE_ACSTR[][12];

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint8_t vtype;
} version_t; // Perfectly fits 32 bits

class LilkaSDK {
public:
    LilkaSDK();

    version_t getVersion();
    const String& getVersionStr();
    version_type_t getVersionType();

private:
    version_t version =
        {.major = SDK_VERSION_MAJOR, .minor = SDK_VERSION_MINOR, .patch = SDK_VERSION_PATCH, .vtype = SDK_VERSION_TYPE};
    ;
    String versionStr;
};

extern LilkaSDK sdk;

} // namespace lilka
