#include "my_qsort.h"

#include "my_swap.h"

void myQsort(void* base, size_t count, size_t size, int (*compare)(const void*, const void*)) {
    if (count < 2) {
        return;
    }

    int flag = 1;
    for (size_t i = 0; flag == 1 && i < count - 1; i++) {
        flag = 0;
        for (size_t j = 0; j < count - 1 - i; j++) {
            void* x = (char*)base + j * size;
            void* y = (char*)base + (j + 1) * size;
            if (compare(x, y)) {
                mySwap(x, y, size);
                flag = 1;
            }
        }
    }
    return;
}