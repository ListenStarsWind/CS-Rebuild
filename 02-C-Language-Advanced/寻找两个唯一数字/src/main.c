#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "find_two_unique_numbers.h"

char* processIteration(char* buf, int n) {
    printf("请输入一行数字, 以空格分割\n");
    printf("如: 10 1 2 3 4 5 1 2 3 4 6\n");
    printf("以回车或者CTRL D 表示该行结束\n");
    printf("输入空行则退出循环\n");
    printf("第一个数字表示样本的个数\n");
    printf("接下来的数字除了两个数外, 其他数都是成对的\n");
    printf("注意: 一行不要超过%d个字符\n", n);
    return fgets(buf, n, stdin);
}

int main(void) {
    int size = 0;
    char buf[64] = {0};
    while (processIteration(buf, sizeof(buf) - 1)) {
        int len = 0;
        len = strlen(buf);
        if(buf[len-1] == '\n'){
            buf[--len] = '\0';
        }

        char* str = buf;
        char* next = NULL;
        long val = strtol(str, &next, 10);
        if (str != next) {
            size = (int)val;
            str = next;
            next = NULL;
        }

        int* arr = malloc(sizeof(int) * size);
        for (int i = 0; i < size; i++) {
            val = strtol(str, &next, 10);
            if (str != next) {
                arr[i] = (int)val;
                str = next;
                next = NULL;
            }
        }

        int num1, num2;
        findTwoUniqueNumbers(arr, size, &num1, &num2);
        printf("两个唯一数为: %d %d\n\n", num1, num2);

        free(arr);
        size = 0;
    }
    return 0;
}
