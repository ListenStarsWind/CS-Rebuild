#include "greatest_common_divisor.h"

int greatestCommonDivisor(int x, int y) {
    int z = 0;
    while ((z = x % y)) {
        x = y;
        y = z;
    }
    return y;
}