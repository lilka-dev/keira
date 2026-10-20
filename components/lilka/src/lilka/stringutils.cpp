#include "stringutils.h"
namespace lilka {
// move cstr_end next Unicode character
char* StringUtils::uforward(char* cstr) {
    // TXT_DBG LEP;
    if (!cstr || !*cstr) return cstr;
    cstr++;
    while ((*cstr & 0xC0) == 0x80)
        cstr++; // skip continuation bytes
    return cstr;
}

// move cstr_end beginning of previous Unicode character
char* StringUtils::ubackward(char* cstr) {
    // TXT_DBG LEP;
    if (!cstr) return cstr;
    char* p = cstr - 1;
    while ((*(reinterpret_cast<unsigned char*>(p)) & 0xC0) == 0x80)
        --p; // skip continuation bytes
    return p;
}

// get length of a first Unicode character
size_t StringUtils::uclen(char* cstr) {
    const char* nextchar = uforward(cstr);
    return nextchar - cstr;
}

// Get length in Unicode characters
size_t StringUtils::ulen(char* cstr, const char* cstr_end) {
    if (!cstr) return 0;

    size_t len = 0;
    char* ptr = cstr;

    if (!cstr_end) {
        while (*ptr) {
            ptr = uforward(ptr);
            len++;
        }
    } else {
        if (cstr_end - cstr <= 0) return 0;
        while (ptr < cstr_end && *ptr) {
            char* next = uforward(ptr);
            if (next > cstr_end) break; // don't go past 'cstr_end'
            ptr = next;
            len++;
        }
    }

    return len;
}

StringUtils sutils;
} // namespace lilka
