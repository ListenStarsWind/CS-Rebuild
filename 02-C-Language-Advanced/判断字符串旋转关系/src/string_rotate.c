#include "string_rotate.h"

#include "greatest_common_divisor.h"

char* rotateString(char* str, int size, int k) {
    k = k % size;
    if (k == 0) return str;

    int gcd = greatestCommonDivisor(size, k);
    int len = size / gcd;
    for (int i = 0; i < gcd; i++) {
        char tmp = str[i];
        int j = i;
        for (int z = 1; z < len; z++) {
            int next = (i + z * k) % size;
            str[j] = str[next];
            j = next;
        }
        str[j] = tmp;
    }
    return str;
}