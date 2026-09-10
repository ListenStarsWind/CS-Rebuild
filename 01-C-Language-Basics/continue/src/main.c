#include <stdio.h>

// 注意 continue 会跳过循环内容， 因此有可能跳过循环变量的调整环节， 导致卡死。
// 结果是 1 2 3 4 然后卡死

int main(void) {
    int i = 1;
    do {
        if (i == 5) continue;
        printf("%d ", i++);
        fflush(stdout);
    } while (i <= 10);

    return 0;
}
