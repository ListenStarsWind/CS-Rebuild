#include <stdio.h>

#include "my_memcpy.h"
#include "system_info.h"

int main(void) {
    printSystemInfo();
    int arr1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int arr2[10] = {0};
    myMemcpy(arr2, arr1, sizeof(arr1));
    for (int i = 0; i < 10; i++) {
        printf("%d ", arr2[i]);
    }
    printf("\n");
    return 0;
}
