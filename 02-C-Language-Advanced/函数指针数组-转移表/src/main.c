#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dispatch_table.h"

char* processIteration(char* buf, int n) {
    printf("接下来你可以输入一行包含+-*/的算式\n");
    printf("例如: \"3.14 + 1.86\", 随后再回车或者按下CTRL D\n");
    printf("注意, 一行不要超过63个字符\n");
    return fgets(buf, n, stdin);
}

int main(void) {
    double x, y, z;
    char op = 0;
    char buf[64] = {0};
    while (processIteration(buf, sizeof(buf))) {
        int size = strlen(buf);
        if (buf[size - 1] == '\n') buf[--size] = '\0';
        char* str = buf;
        char* next = NULL;
        double val = 0.0;
        val = strtod(str, &next);
        if (str != next) {
            x = val;
            str = next;
            next = NULL;
        }

        while (*str == ' ') ++str;

        op = *str++;

        val = strtod(str, &next);
        if (str != next) {
            y = val;
            str = next;
            next = NULL;
        }

        int flag = 0;
        switch (op) {
            case '+':
                z = operationTable[0](x, y);
                break;
            case '-':
                z = operationTable[1](x, y);
                break;
            case '*':
                z = operationTable[2](x, y);
                break;
            case '/':
                z = operationTable[3](x, y);
                break;
            default:
                flag = 1;
                break;
        }
        if (flag)
            printf("出现未识别操作符\n");
        else
            printf("%.15g %c %.15g = %.15g\n", x, op, y, z);
        printf("\n");
    }
    return 0;
}
