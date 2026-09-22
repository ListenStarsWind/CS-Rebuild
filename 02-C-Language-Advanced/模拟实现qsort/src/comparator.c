#include "comparator.h"

int isIntGreater(const void* lhs, const void* rhs){
    int* x = (int*)lhs;
    int* y = (int*)rhs;
    return *x > *y;
}