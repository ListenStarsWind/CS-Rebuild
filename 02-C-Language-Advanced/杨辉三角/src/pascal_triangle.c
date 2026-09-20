#include "pascal_triangle.h"
#include <stdlib.h>

int** generate(int numRows, int* returnSize, int** returnColumnSizes) {
    int** arr = malloc(sizeof(int*) * numRows);
    *returnColumnSizes = malloc(sizeof(int)*numRows);
    for(int i = 0; i < numRows; i++){
        arr[i] = malloc(sizeof(int)*(i+1));
    }
    for(int i = 0; i < numRows; i++){
        arr[i][0] = 1;
        for(int j = 1; j < i; j++)
            arr[i][j] = arr[i-1][j-1]+arr[i-1][j];
        arr[i][i] = 1;
    }
    *returnSize = numRows;
    for(int i = 0; i < numRows; i++){
        (*returnColumnSizes)[i] = i + 1;
    }
    return arr;
}