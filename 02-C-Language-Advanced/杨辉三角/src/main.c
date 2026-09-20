#include <stdio.h>
#include <stdlib.h>

#include "pascal_triangle.h"

int main(void) {
    int numRows = 0;
    scanf(" %d", &numRows);
    int returnSize = 0;
    int* returnColumnSizes = NULL;
    int** arr = generate(numRows, &returnSize, &returnColumnSizes);
    printf("[");
    for (int i = 0; i < returnSize; i++) {
        printf("[");
        for(int j = 0; j < returnColumnSizes[i]; j++){
            printf("%d", arr[i][j]);
            if(j < returnColumnSizes[i] - 1) printf(",");
        }
        printf("]");
        if (i < returnSize - 1) printf(",");
    }
    printf("]\n");

    for(int i = 0; i < returnSize; i++)
        free(arr[i]);
    free(arr);
    free(returnColumnSizes);
    return 0;
}
