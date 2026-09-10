#include <stdio.h>

int func(int num) {
    return (num % 4 == 0 && num % 100 != 0) || num % 400 == 0;
}

int main(void) {
    int cur = 0;
    int end = 0;
    printf("输入起始及终止年份:");
    while (scanf("%d", &cur) != 1);
    while (scanf("%d", &end) != 1);
    for (; cur <= end; cur++) {
        if (func(cur)) printf("%d ", cur);
    }
    printf("\n");
    return 0;
}
