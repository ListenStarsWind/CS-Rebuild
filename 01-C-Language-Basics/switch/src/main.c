#include <stdio.h>

int main(void) {
    int a = 0;
    while (scanf("%d", &a) == EOF);
    switch (a) {
        case 0:
            printf("他是0\n");
            break;
        case 1:
            printf("他是1\n");
            break;
        default:
            printf("说实话， 我不想再写这些东西了\n");
            break;
        case 2:
            printf("他是2\n");
            break;
        case 3:
            printf("他是3\n");
            break;
        case 4:
            printf("他是4\n");
        case 5:
            printf("他是5\n");
            break;
    }

    return 0;
}
