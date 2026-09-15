#include <stdio.h>

void swap_with_temp(int* a, int* b) {
    int tmp = *b;
    *b = *a;
    *a = tmp;
}

void swap_with_arithmetic(int* a, int* b) {
    *a = *a + *b;
    *b = *a - *b;
    *a = *a - *b;
}

void swap_with_xor(int* a, int* b) {
    *a = *a ^ *b;
    *b = *a ^ *b;
    *a = *a ^ *b;
}

int main(void) {
    int a[3] = {4, 7, 9};
    int b[3] = {2, 6, 3};
    for (int i = 0; i < 3; i++) printf("%d ", a[i]);
    printf("\n");
    for (int i = 0; i < 3; i++) printf("%d ", b[i]);
    printf("\n");
    swap_with_temp(a, b);
    swap_with_arithmetic(a + 1, b + 1);
    swap_with_xor(a + 2, b + 2);
    for (int i = 0; i < 3; i++) printf("%d ", a[i]);
    printf("\n");
    for (int i = 0; i < 3; i++) printf("%d ", b[i]);
    printf("\n");
    return 0;
}
