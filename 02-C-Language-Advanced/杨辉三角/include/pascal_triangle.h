#pragma once

// numRows 决定展示的杨辉三角行数, 返回一个二维数组, returnSize 输出二维数组最外层元素个数
// returnColumSize 输出一个一维数组, 其元素描述返回二维数组第二层, 也就是各行元素个数
int** generate(int numRows, int* returnSize, int** returnColumnSizes);