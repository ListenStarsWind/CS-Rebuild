#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// 你的待测函数
bool searchInSortedMatrix(
    int m,
    int n,
    int arr[m][n],
    int target
);

// 基准算法：暴力扫描
static bool searchInSortedMatrixByBruteForce(
    int m,
    int n,
    int arr[m][n],
    int target
)
{
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (arr[i][j] == target)
                return true;
        }
    }

    return false;
}

// 生成一个严格满足行、列递增的矩阵
static void generateSortedMatrix(
    int m,
    int n,
    int arr[m][n]
)
{
    arr[0][0] = rand() % 20 - 10;

    // 第一行
    for (int j = 1; j < n; ++j) {
        arr[0][j] = arr[0][j - 1] + rand() % 5 + 1;
    }

    // 后面的行
    for (int i = 1; i < m; ++i) {
        arr[i][0] = arr[i - 1][0] + rand() % 5 + 1;

        for (int j = 1; j < n; ++j) {
            int lowerBound =
                arr[i - 1][j] > arr[i][j - 1]
                ? arr[i - 1][j]
                : arr[i][j - 1];

            arr[i][j] = lowerBound + rand() % 5 + 1;
        }
    }
}

static void printMatrix(
    int m,
    int n,
    int arr[m][n]
)
{
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%5d", arr[i][j]);
        }
        putchar('\n');
    }
}

int main(void)
{
    srand((unsigned)time(NULL));

    const int testCount = 1000;
    const int queryCount = 100;
    const int maxRows = 50;
    const int maxCols = 50;

    int passed = 0;

    printf("开始随机测试...\n");
    printf("矩阵测试次数: %d\n", testCount);
    printf("每个矩阵查询次数: %d\n", queryCount);
    printf("最大矩阵尺寸: %d x %d\n\n", maxRows, maxCols);

    for (int test = 1; test <= testCount; ++test) {

        int m = rand() % maxRows + 1;
        int n = rand() % maxCols + 1;

        int (*arr)[n] = malloc(sizeof *arr * m);

        if (arr == NULL) {
            fprintf(stderr, "内存分配失败\n");
            return 1;
        }

        generateSortedMatrix(m, n, arr);

        for (int q = 0; q < queryCount; ++q) {

            int target;

            // 一半查询矩阵中真实存在的元素
            if (rand() % 2 == 0) {
                int i = rand() % m;
                int j = rand() % n;

                target = arr[i][j];
            }
            // 一半查询随机数字
            else {
                int min = arr[0][0] - 20;
                int max = arr[m - 1][n - 1] + 20;

                target = min + rand() % (max - min + 1);
            }

            bool expected =
                searchInSortedMatrixByBruteForce(
                    m, n, arr, target
                );

            bool actual =
                searchInSortedMatrix(
                    m, n, arr, target
                );

            if (expected != actual) {

                printf("\n测试失败\n");
                printf("测试编号: %d\n", test);
                printf("查询编号: %d\n", q + 1);
                printf("矩阵大小: %d x %d\n", m, n);
                printf("目标数字: %d\n", target);
                printf("正确结果: %s\n",
                       expected ? "存在" : "不存在");
                printf("实际结果: %s\n",
                       actual ? "存在" : "不存在");

                printf("\n矩阵:\n");
                printMatrix(m, n, arr);

                free(arr);
                return 1;
            }
        }

        free(arr);
        ++passed;

        if (test % 100 == 0) {
            printf("已通过 %d 组矩阵测试\n", test);
        }
    }

    printf("\n================================\n");
    printf("随机测试完成\n");
    printf("通过: %d\n", passed);
    printf("失败: 0\n");
    printf("总查询次数: %d\n", testCount * queryCount);
    printf("================================\n");

    return 0;
}