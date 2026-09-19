#include <stdio.h>
#include <stdlib.h>
#include "bubble_sort.h"

int main(void) {
    int* arr = NULL;
    int size = 0;
    printf("确定数组元素数目: ");
    scanf(" %d", &size);
    arr = (int*)malloc(sizeof(int) * size);
    if (arr != NULL) {
        printf("现在初始化数组: ");
        for (int i = 0; i < size; i++) {
            scanf(" %d", arr + i);
        }
        bubbleSort(arr, size);
        printf("排序后的结果为: ");
        for (int i = 0; i < size; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
    }
    free(arr);
    arr = NULL;
    return 0;
}
