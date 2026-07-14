#include "string.h"
#include <ctype.h>

String StringFormat(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t size = vsnprintf(nullptr, 0, fmt, args) + 1; // +1 for null-terminator
    va_end(args);

    char* buffer = new char[size];
    if (!buffer) return String(); // allocation failed

    va_start(args, fmt);
    vsnprintf(buffer, size, fmt, args);
    va_end(args);

    String result(buffer);
    delete[] buffer;
    return result;
}

bool strstrends(const char* haystack, const char* needle) {
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    if (nlen > hlen) return false;
    return memcmp(haystack + hlen - nlen, needle, nlen) == 0;
}

bool strcasestrends(const char* haystack, const char* needle) {
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    if (nlen > hlen) return false;
    const char* tail = haystack + hlen - nlen;
    for (size_t i = 0; i < nlen; i++) {
        if (tolower((unsigned char)tail[i]) != tolower((unsigned char)needle[i])) return false;
    }
    return true;
}
