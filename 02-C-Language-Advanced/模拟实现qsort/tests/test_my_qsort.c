#include "my_qsort.h"
#include "comparator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEST_COUNT 10000
#define MAX_SIZE 100
#define VALUE_RANGE 1000

static int compareIntForQsort(const void* lhs, const void* rhs)
{
    int x = *(const int*)lhs;
    int y = *(const int*)rhs;

    return (x > y) - (x < y);
}

static int arraysEqual(const int* lhs, const int* rhs, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (lhs[i] != rhs[i]) {
            return 0;
        }
    }

    return 1;
}

static void printArray(const int* arr, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }

    printf("\n");
}

int main(void)
{
    srand((unsigned)time(NULL));

    printf("开始随机测试...\n");
    printf("测试次数: %d\n", TEST_COUNT);
    printf("最大数组长度: %d\n\n", MAX_SIZE);

    for (int test = 1; test <= TEST_COUNT; test++) {
        size_t size = (size_t)(rand() % (MAX_SIZE + 1));

        int original[MAX_SIZE];
        int actual[MAX_SIZE];
        int expected[MAX_SIZE];

        for (size_t i = 0; i < size; i++) {
            original[i] =
                rand() % (VALUE_RANGE * 2 + 1) - VALUE_RANGE;
        }

        memcpy(actual, original, size * sizeof(int));
        memcpy(expected, original, size * sizeof(int));

        myQsort(
            actual,
            size,
            sizeof(int),
            isIntGreater
        );

        qsort(
            expected,
            size,
            sizeof(int),
            compareIntForQsort
        );

        if (!arraysEqual(actual, expected, size)) {
            printf("================================\n");
            printf("测试失败: 第 %d 组\n", test);
            printf("数组长度: %zu\n", size);

            printf("原数组:   ");
            printArray(original, size);

            printf("myQsort:  ");
            printArray(actual, size);

            printf("标准qsort:");
            printArray(expected, size);

            printf("================================\n");

            return EXIT_FAILURE;
        }

        if (test % 1000 == 0) {
            printf("已通过 %d 组\n", test);
        }
    }

    printf("\n================================\n");
    printf("随机测试完成\n");
    printf("通过: %d\n", TEST_COUNT);
    printf("失败: 0\n");
    printf("================================\n");

    return EXIT_SUCCESS;
}