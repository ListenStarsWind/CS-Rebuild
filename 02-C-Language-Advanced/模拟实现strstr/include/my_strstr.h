#pragma once

#include <stdbool.h>

char* myStrstrImpl(
    const char* haystack,
    const char* needle,
    bool useKmp
);

#ifdef USE_KMP_STRSTR

#define myStrstr(haystack, needle) \
    myStrstrImpl((haystack), (needle), true)

#else

#define myStrstr(haystack, needle) \
    myStrstrImpl((haystack), (needle), false)

#endif
