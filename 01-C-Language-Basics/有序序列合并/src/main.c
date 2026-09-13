#include <stdio.h>

int main(void) {
    int arr1[1000] = {0};
    int arr2[1000] = {0};
    int arr3[1000] = {0};
    int size1 = 0, size2 = 0;
    scanf("%d%d", &size1, &size2);
    for (int i = 0; i < size1; i++) scanf("%d", arr1 + i);
    for (int i = 0; i < size2; i++) scanf("%d", arr2 + i);
    int first = 0, second = 0, dest = 0;
    while (first < size1 && second < size2) {
        if (arr1[first] < arr2[second]) {
            arr3[dest++] = arr1[first++];
        } else {
            arr3[dest++] = arr2[second++];
        }
    }
    while (first < size1) arr3[dest++] = arr1[first++];
    while (second < size2) arr3[dest++] = arr2[second++];
    for (int i = 0; i < dest; i++) printf("%d ", arr3[i]);
    printf("\n");
    return 0;
}
