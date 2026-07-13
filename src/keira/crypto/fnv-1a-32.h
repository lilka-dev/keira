#pragma once

#include <esp_attr.h>
//////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
//////////////////////////////////////////////////////////////////////////////
// Fowler/Noll/Vo 32-bit FNV-1a hash
//////////////////////////////////////////////////////////////////////////////
// Stripped to 32-bit FNV-1a only. Original algorithm by:
//   Phong Vo       (http://www.research.att.com/info/kpv/)
//   Glenn Fowler   (http://www.research.att.com/~gsf/)
//   Landon C. Noll (http://www.isthe.com/chongo/tech/comp/fnv/)
//
// Public domain. See: http://www.isthe.com/chongo/tech/comp/fnv/
//////////////////////////////////////////////////////////////////////////////

// To be used in situations where crc32 useless(small data amount)

#define FNV_1A_INITIAL_HVAL 0x811C9DC5

static inline IRAM_ATTR uint32_t fnv_32a_buf(const void* buf, size_t len, uint32_t hval) {
    const unsigned char* bp = (const unsigned char*)buf;
    const unsigned char* be = bp + len;

    while (bp < be) {
        hval ^= (uint32_t)*bp++;

        // TODO: measure speed of both
        hval *= 0x01000193;
        // and it's alternative
        //hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
    }

    return hval;
}

/* Null-terminated string variant */
static inline IRAM_ATTR uint32_t fnv_32a_cstr(const char* str, uint32_t hval) {
    const unsigned char* s = (const unsigned char*)str;

    while (*s) {
        hval ^= (uint32_t)*s++;

        // TODO: measure speed of both
        hval *= 0x01000193;
        // and it's alternative
        //hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
    }

    return hval;
}