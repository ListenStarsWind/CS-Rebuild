#include "my_strncpy.h"

char* myStrncpy(char* dest, const char* src, size_t size) {
    char* ret = dest;
    size_t i = 0;
    while (src[i] != '\0' && i < size) {
        dest[i] = src[i];
        i++;
    }
    while (i < size) dest[i++] = '\0';
    return ret;
}