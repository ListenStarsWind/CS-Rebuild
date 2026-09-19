#include "int_swap.h"

void int_swap(int* x, int* y) {
    int tmp = *x;
    *x = *y;
    *y = tmp;
}