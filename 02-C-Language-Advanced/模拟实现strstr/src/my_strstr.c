#include "my_strstr.h"

#include <stdlib.h>
#include <string.h>

static char* kmpStrstr(const char* haystack, size_t len1, const char* needle, size_t len2) {
    (void)len1;
    size_t* lps = malloc(len2 * sizeof *lps);
    if (lps == NULL) return NULL;

    lps[0] = 0;

    for (size_t i = 1; i < len2; ++i) {
        size_t m = lps[i - 1];

        while (m > 0 && needle[m] != needle[i]) {
            m = lps[m - 1];
        }

        if (needle[m] == needle[i]) {
            ++m;
        }

        lps[i] = m;
    }

    size_t i = 0;
    size_t j = 0;

    while (haystack[i] != '\0') {
        if (haystack[i] == needle[j]) {
            ++i;
            ++j;

            if (j == len2) {
                free(lps);
                return (char*)haystack + (i - j);
            }
        } else if (j > 0) {
            j = lps[j - 1];
        } else {
            ++i;
        }
    }

    free(lps);
    return NULL;
}

static char* naiveStrstr(const char* haystack, size_t len1, const char* needle, size_t len2) {
    size_t i = 0;
    size_t j = 0;
    for (; i + len2 <= len1; i++) {
        size_t x = i;
        size_t y = j;
        while (y != len2 && haystack[x] == needle[y]) {
            x++;
            y++;
        }
        if (y != len2 && haystack[x] != needle[y]) {
            j = 0;
        } else {
            return (char*)haystack + i;
        }
    }

    return NULL;
}

char* myStrstrImpl(const char* haystack, const char* needle, bool useKmp) {
    size_t len1 = strlen(haystack);
    size_t len2 = strlen(needle);

    if (len2 == 0) return (char*)haystack;

    if (len2 > len1) return NULL;

    if (useKmp) return kmpStrstr(haystack, len1, needle, len2);

    return naiveStrstr(haystack, len1, needle, len2);
}