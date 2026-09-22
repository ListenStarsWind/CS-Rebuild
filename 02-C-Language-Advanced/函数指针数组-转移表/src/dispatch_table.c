#include "dispatch_table.h"

#include <math.h>

double add(double x, double y) {
    return x + y;
}

double sub(double x, double y) {
    return x - y;
}

double mul(double x, double y) {
    return x * y;
}

double idiv(double x, double y) {
    if (y == 0) return INFINITY;
    return x / y;
}

double (*operationTable[])(double, double) = {add, sub, mul, idiv};