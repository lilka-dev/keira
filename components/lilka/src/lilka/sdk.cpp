#include "sdk.h"

namespace lilka {
LilkaSDK::LilkaSDK() {
    // Prepare all data at once and reuse
    char tmpBuffer[260] = {};
    sprintf(
        tmpBuffer,
        "LilkaSDK v%d.%d.%d %s",
        version.major,
        version.minor,
        version.patch,
        SDK_VERSION_TYPE_ACSTR[version.vtype]
    );
    versionStr = tmpBuffer;
}

version_t LilkaSDK::getVersion() {
    return version;
}

const String& LilkaSDK::getVersionStr() {
    return versionStr;
}

char SDK_VERSION_TYPE_ACSTR[][12] = {"Dev", "Pre-Release", "Release"};
LilkaSDK sdk;
} // namespace lilka
