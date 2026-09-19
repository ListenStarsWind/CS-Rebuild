#include "bubble_sort.h"
#include "int_swap.h"

void bubbleSort(int* arr, int size) {
    int flag = 1;
    for (int i = 0; flag == 1 && i < size - 1; i++) {
        flag = 0;
        for (int j = 0; j < size - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                flag = 1;
                int_swap(arr + j, arr + j + 1);
            }
        }
    }
}
