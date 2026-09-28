#include <stdio.h>
#include <string.h>

#include "system_info.h"

int main(void) {
    printSystemInfo();
    int arr1[] = {1, 2, 3, 4, 5, 6, 7, 8};
    int arr2[] = {1, 2, 3, 4, 8, 8, 8, 8};
    int ret = memcmp(arr1, arr2, 5 * sizeof(int));
    printf("%d\n", ret);
    return 0;
}
