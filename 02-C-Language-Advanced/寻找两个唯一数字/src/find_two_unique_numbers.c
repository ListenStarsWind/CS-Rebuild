#include "find_two_unique_numbers.h"

void findTwoUniqueNumbers(const int* arr, int size, int* num1, int* num2) {
    int diff = 0;
    for (int i = 0; i < size; i++) {
        diff ^= arr[i];
    }

    int key = 0;
    for (int i = 0; i < 32; i++) {
        if ((diff >> i) & 1) {
            key = i;
            break;
        }
    }

    int nums1 = 0;
    int nums2 = 0;
    for (int i = 0; i < size; i++) {
        if ((arr[i] >> key) & 1)
            nums1 ^= arr[i];
        else
            nums2 ^= arr[i];
    }

    *num1 = nums1;
    *num2 = nums2;

    return;
}