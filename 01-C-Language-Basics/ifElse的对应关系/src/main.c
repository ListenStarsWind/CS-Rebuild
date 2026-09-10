#include <stdio.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    int a = 0;
    int b = 1;

    // else 与 最近if 匹配
    if (a == 1)
        if (b == 1)
            printf("haha");
        else
            printf("hehe");
    printf("\n");
    return 0;
}
