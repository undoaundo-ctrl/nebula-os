#pragma once
#include <stddef.h>
#include <stdint.h>

namespace libk {

inline void* memset(void* dst, int val, size_t n) {
    uint8_t* d = static_cast<uint8_t*>(dst);
    for (size_t i = 0; i < n; i++) d[i] = static_cast<uint8_t>(val);
    return dst;
}

inline void* memcpy(void* dst, const void* src, size_t n) {
    uint8_t* d = static_cast<uint8_t*>(dst);
    const uint8_t* s = static_cast<const uint8_t*>(src);
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dst;
}

inline void* memmove(void* dst, const void* src, size_t n) {
    uint8_t* d = static_cast<uint8_t*>(dst);
    const uint8_t* s = static_cast<const uint8_t*>(src);
    if (d < s) {
        for (size_t i = 0; i < n; i++) d[i] = s[i];
    } else {
        for (size_t i = n; i-- > 0;) d[i] = s[i];
    }
    return dst;
}

inline size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

inline int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *reinterpret_cast<const unsigned char*>(a) -
           *reinterpret_cast<const unsigned char*>(b);
}

} // namespace libk
