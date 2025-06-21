#include "keira.h"

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
