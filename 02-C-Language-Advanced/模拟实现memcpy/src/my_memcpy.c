#include "my_memcpy.h"

void* myMemcpy(void* dest, void* src, size_t size) {
    for (size_t i = 0; i < size; i++) {
        *((char*)dest + i) = *((char*)src + i);
    }
    return dest;
}