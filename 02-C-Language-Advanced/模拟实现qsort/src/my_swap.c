#include "my_swap.h"
#include <stdlib.h>
#include <string.h>

void mySwap(void* x, void* y, size_t size){
    char* tmp = malloc(size);
    memcpy(tmp, x, size);
    memcpy(x, y, size);
    memcpy(y, tmp, size);
    free(tmp);
}