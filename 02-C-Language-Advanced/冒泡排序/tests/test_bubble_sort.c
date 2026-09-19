#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bubble_sort.h"


#define RANDOM_TEST_COUNT 10000
#define MAX_ARRAY_SIZE    100
#define MIN_VALUE        -1000
#define MAX_VALUE         1000


static int compareInt(const void* lhs, const void* rhs)
{
    const int a = *(const int*)lhs;
    const int b = *(const int*)rhs;

    return (a > b) - (a < b);
}


static int checkOneTest(const int* source, int size, int testNumber)
{
    int actual[MAX_ARRAY_SIZE];
    int expected[MAX_ARRAY_SIZE];

    memcpy(actual, source, (size_t)size * sizeof(int));
    memcpy(expected, source, (size_t)size * sizeof(int));

    bubbleSort(actual, size);

    qsort(
        expected,
        (size_t)size,
        sizeof(int),
        compareInt
    );


    for (int i = 0; i < size; ++i) {

        if (actual[i] != expected[i]) {

            printf("\n测试失败\n");
            printf("测试编号: %d\n", testNumber);
            printf("数组长度: %d\n", size);

            printf("\n原始数组:\n");

            for (int j = 0; j < size; ++j)
                printf("%d ", source[j]);

            printf("\n\n期望结果:\n");

            for (int j = 0; j < size; ++j)
                printf("%d ", expected[j]);

            printf("\n\n实际结果:\n");

            for (int j = 0; j < size; ++j)
                printf("%d ", actual[j]);

            printf("\n");

            return 0;
        }
    }


    return 1;
}


int main(void)
{
    /*
     * 固定种子。
     *
     * 一旦随机测试失败，可以再次得到同一批数据，
     * 方便复现问题。
     */
    srand(20260919);


    /*
     * --------------------------------------------------
     * 固定边界测试
     * --------------------------------------------------
     */

    {
        int arr[] = {42};

        if (!checkOneTest(arr, 1, 0))
            return EXIT_FAILURE;
    }


    {
        int arr[] = {2, 1};

        if (!checkOneTest(arr, 2, 0))
            return EXIT_FAILURE;
    }


    {
        int arr[] = {1, 2, 3, 4, 5, 6, 7, 8};

        if (!checkOneTest(arr, 8, 0))
            return EXIT_FAILURE;
    }


    {
        int arr[] = {8, 7, 6, 5, 4, 3, 2, 1};

        if (!checkOneTest(arr, 8, 0))
            return EXIT_FAILURE;
    }


    {
        int arr[] = {5, 2, 8, 2, 5, 1, 8, 3, 3, 5};

        if (!checkOneTest(arr, 10, 0))
            return EXIT_FAILURE;
    }


    {
        int arr[] = {3, -5, 0, 8, -1, 4, -10, 2};

        if (!checkOneTest(arr, 8, 0))
            return EXIT_FAILURE;
    }


    /*
     * --------------------------------------------------
     * 随机测试
     * --------------------------------------------------
     */

    for (int test = 1; test <= RANDOM_TEST_COUNT; ++test) {

        int size = rand() % MAX_ARRAY_SIZE + 1;

        int arr[MAX_ARRAY_SIZE];


        for (int i = 0; i < size; ++i) {

            arr[i] =
                rand() % (MAX_VALUE - MIN_VALUE + 1)
                + MIN_VALUE;
        }


        if (!checkOneTest(arr, size, test))
            return EXIT_FAILURE;


        if (test % 1000 == 0)
            printf("已通过 %d 组随机测试\n", test);
    }


    printf("\n");
    printf("================================\n");
    printf("冒泡排序测试完成\n");
    printf("固定测试: 全部通过\n");
    printf("随机测试: %d 组全部通过\n", RANDOM_TEST_COUNT);
    printf("================================\n");


    return EXIT_SUCCESS;
}
