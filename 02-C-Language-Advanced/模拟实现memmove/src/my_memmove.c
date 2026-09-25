#include "my_memmove.h"

void* myMemmove(void* dest, void* src, size_t size) {
    void* ret = dest;
    if (dest < src && (char*)src < (char*)dest + size) {
        // 顺着读
        for (size_t i = 0; i < size; i++) {
            *((char*)dest + i) = *((char*)src + i);
        }
    } else {
        // 倒着读
        while (size--) {
            *((char*)dest + size) = *((char*)src + size);
        }
    }
    return ret;
}