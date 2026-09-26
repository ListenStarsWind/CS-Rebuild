#include "my_strcmp.h"

int myStrcmp(const char* s1, const char* s2) {
    while (*s1 != '\0' && *s2 != '\0') {
        int cmp = *s1 - *s2;
        if (cmp != 0) return cmp;
        s1++, s2++;
    }
    return *s1 - *s2;
}