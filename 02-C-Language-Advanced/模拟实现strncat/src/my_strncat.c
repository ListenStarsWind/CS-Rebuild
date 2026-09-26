#include "my_strncat.h"

#include <string.h>

char* myStrncat(char* dst, const char* src, size_t size) {
    char* ret = dst;

    size_t i = 0;
    size_t len = strlen(dst);
    while (src[i] != '\0' && i < size) {
        dst[len++] = src[i++];
    }
    dst[len] = '\0';

    return ret;
}