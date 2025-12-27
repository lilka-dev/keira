#ifndef VERSION_KEIRA_H
#define VERSION_KEIRA_H

#include <string>
#include "keira_lang.h"
#include <lilka.h>
#include "keira_version_auto_gen.h"

enum KEIRA_VERSION_TYPE : uint8_t {
    KEIRA_VERSION_TYPE_DEV = 0,
    KEIRA_VERSION_TYPE_PRE_RELEASE = 1,
    KEIRA_VERSION_TYPE_RELEASE = 2
};

int KEIRA_get_curr_version_type() {
    return KEIRA_VERSION_TYPE;
}

lilka::version_t KEIRA_get_curr_version() {
    lilka::version_t version = {
        .major = KEIRA_VERSION_MAJOR,
        .minor = KEIRA_VERSION_MINOR,
        .patch = KEIRA_VERSION_PATCH,
        .version_type = KEIRA_VERSION_TYPE
    };
    return version;
}

String KEIRA_get_firmware_languge() {
    return K_S_CURRENT_LANGUAGE_SHORT;
}

#endif