#include <stdio.h>

int count_ones_by_division(unsigned int n) {
    int ret = 0;
    while (n) {
        if (n % 2) ret++;
        n /= 2;
    }
    return ret;
}

int count_ones_by_bit_scan(unsigned int n) {
    int ret = 0;
    for (int i = 0; i < 32; i++) {
        if (n & 1) ret++;
        n >>= 1;
    }
    return ret;
}

int count_ones_by_clearing(unsigned int n) {
    int ret = 0;
    while (n) {
        ret++;
        n &= n - 1;
    }
    return ret;
}

int main(void) {
    int arr[10] = {0};

    for (int i = 0; i < 10; i++) scanf("%d", arr + i);

    printf("\n\n");     // 先让终端真正向下滚动，预留两行
    printf("\033[2A");  // 回到三行区域的第一行

    for (int i = 0; i < 10; i++) {
        printf("\033[s");  // 保存当前列起点

        // 第一行
        printf("%d", count_ones_by_division(arr[i]));

        // 回到列起点，再下移一行
        printf("\033[u");
        printf("\033[1B");
        printf("%d", count_ones_by_bit_scan(arr[i]));

        // 再次回到列起点，下移两行
        printf("\033[u");
        printf("\033[2B");
        printf("%d", count_ones_by_clearing(arr[i]));

        // 回到第一行的列起点，准备下一列
        printf("\033[u");
        printf("\033[4C");
    }

    // 当前光标还在第一行，移到三行数据下面
    printf("\033[3B\r\n");

    return 0;
}