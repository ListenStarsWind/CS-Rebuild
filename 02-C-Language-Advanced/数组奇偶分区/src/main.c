#include <stdio.h>
#include <string.h>

// 返回奇数的数目
int partitionByTwoPointers(int* arr, int size) {
    if (size == 0) return 0;

    int left = 0;
    int right = size - 1;
    while (left < right) {
        while (left < right && arr[left] % 2 != 0) left++;
        while (left < right && arr[right] % 2 == 0) right--;
        int tmp = arr[left];
        arr[left] = arr[right];
        arr[right] = tmp;
    }
    return left + (arr[left] % 2 != 0);
}

int partitionByForwardScan(int* arr, int size) {
    int boundary = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] % 2 != 0) {
            int tmp = arr[i];
            arr[i] = arr[boundary];
            arr[boundary++] = tmp;
        }
    }
    return boundary;
}

int partitionByStableInsertion(int* arr, int size){
        int bou = 0;
        for(int i = 0; i < size; i++){
            if(arr[i] % 2 != 0){
                int tmp = arr[i];
                for(int j = i; j > bou; j--){
                    arr[j] = arr[j-1];
                }
                arr[bou++] = tmp;
            }
        }
        return bou;
}

    int main(void) {
    int arr1[10] = {0};
    int arr2[10] = {0};
    int (*pf[3])(int*, int) = {partitionByTwoPointers, partitionByForwardScan, partitionByStableInsertion}; 
    for (int i = 0; i < 10; i++) {
        scanf(" %d", arr1 + i);
    }
    for(int i = 0; i < 3; i++){
        printf("方法%d: ", i);
        memcpy(arr2, arr1, sizeof(arr1));
        int ret = pf[i](arr2, 10);
        for(int j = 0; j < 10; j++)
            printf("%d ", arr2[j]);
        printf("奇数个数: %d\n", ret);
    }
    printf("\n");
    return 0;
}
