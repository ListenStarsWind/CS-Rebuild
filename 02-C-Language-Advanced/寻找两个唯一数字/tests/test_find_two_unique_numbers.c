#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "find_two_unique_numbers.h"

#define RANDOM_TEST_COUNT 10000
#define MAX_PAIRS 50
#define MAX_SIZE (MAX_PAIRS * 2 + 2)

static int compareInt(const void* a, const void* b) {
    int x = *(const int*)a;
    int y = *(const int*)b;

    return (x > y) - (x < y);
}

static int sameTwoNumbers(int a1, int a2, int b1, int b2) {
    return (a1 == b1 && a2 == b2) ||
           (a1 == b2 && a2 == b1);
}

static void printArray(const int* arr, int size) {
    printf("{ ");

    for (int i = 0; i < size; ++i) {
        printf("%d", arr[i]);

        if (i != size - 1)
            printf(", ");
    }

    printf(" }\n");
}

/*
 * 独立参照实现。
 *
 * 将数组排序后，成对数字必然相邻。
 * 无法与后一个数字配成一对的，就是唯一数。
 */
static void referenceFind(
    const int* arr,
    int size,
    int* num1,
    int* num2
) {
    int copy[MAX_SIZE];

    memcpy(copy, arr, (size_t)size * sizeof(int));

    qsort(
        copy,
        (size_t)size,
        sizeof(int),
        compareInt
    );

    int unique[2];
    int count = 0;

    for (int i = 0; i < size;) {

        if (i + 1 < size &&
            copy[i] == copy[i + 1]) {

            i += 2;
            continue;
        }

        if (count < 2)
            unique[count++] = copy[i];

        ++i;
    }

    if (count != 2) {
        fprintf(stderr,
                "测试数据生成错误: 找到 %d 个唯一数\n",
                count);
        exit(EXIT_FAILURE);
    }

    *num1 = unique[0];
    *num2 = unique[1];
}

static void checkCase(
    const int* arr,
    int size,
    int expected1,
    int expected2,
    const char* name
) {
    int actual1 = 0;
    int actual2 = 0;

    findTwoUniqueNumbers(
        arr,
        size,
        &actual1,
        &actual2
    );

    if (!sameTwoNumbers(
            actual1,
            actual2,
            expected1,
            expected2)) {

        fprintf(stderr,
                "\n固定测试失败: %s\n",
                name);

        fprintf(stderr, "size = %d\n", size);

        printf("原数组: ");
        printArray(arr, size);

        fprintf(stderr,
                "期望结果: %d %d\n",
                expected1,
                expected2);

        fprintf(stderr,
                "实际结果: %d %d\n",
                actual1,
                actual2);

        exit(EXIT_FAILURE);
    }
}

static void fixedTests(void) {

    {
        int arr[] = {5, 6};

        checkCase(
            arr,
            2,
            5,
            6,
            "只有两个元素"
        );
    }

    {
        int arr[] = {
            1, 2, 1, 3
        };

        checkCase(
            arr,
            4,
            2,
            3,
            "单组重复数字"
        );
    }

    {
        int arr[] = {
            1, 2, 3, 4, 5,
            1, 2, 3, 4, 6
        };

        checkCase(
            arr,
            10,
            5,
            6,
            "题目示例"
        );
    }

    {
        int arr[] = {
            -1, 4, -1, 7, 9, 9
        };

        checkCase(
            arr,
            6,
            4,
            7,
            "包含负数"
        );
    }

    {
        int arr[] = {
            0, 1, 2, 1, 3, 3
        };

        checkCase(
            arr,
            6,
            0,
            2,
            "包含零"
        );
    }

    {
        int arr[] = {
            INT_MIN,
            7,
            8,
            7,
            INT_MAX,
            8
        };

        checkCase(
            arr,
            6,
            INT_MIN,
            INT_MAX,
            "整数边界值"
        );
    }
}

static int contains(
    const int* values,
    int count,
    int value
) {
    for (int i = 0; i < count; ++i) {
        if (values[i] == value)
            return 1;
    }

    return 0;
}

static int randomValue(void) {
    return rand() % 200001 - 100000;
}

static void shuffle(int* arr, int size) {
    for (int i = size - 1; i > 0; --i) {
        int j = rand() % (i + 1);

        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

static int generateRandomCase(int* arr) {

    int pairCount =
        rand() % (MAX_PAIRS + 1);

    int distinctCount =
        pairCount + 2;

    int values[MAX_PAIRS + 2];

    for (int i = 0; i < distinctCount; ++i) {

        int value;

        do {
            value = randomValue();
        } while (contains(values, i, value));

        values[i] = value;
    }

    /*
     * 前两个值只出现一次。
     */
    int size = 0;

    arr[size++] = values[0];
    arr[size++] = values[1];

    /*
     * 剩余所有值都出现两次。
     */
    for (int i = 2;
         i < distinctCount;
         ++i) {

        arr[size++] = values[i];
        arr[size++] = values[i];
    }

    shuffle(arr, size);

    return size;
}

static void randomTests(void) {

    srand(20260919);

    for (int test = 1;
         test <= RANDOM_TEST_COUNT;
         ++test) {

        int arr[MAX_SIZE];

        int size =
            generateRandomCase(arr);

        int expected1;
        int expected2;

        int actual1 = 0;
        int actual2 = 0;

        referenceFind(
            arr,
            size,
            &expected1,
            &expected2
        );

        findTwoUniqueNumbers(
            arr,
            size,
            &actual1,
            &actual2
        );

        if (!sameTwoNumbers(
                actual1,
                actual2,
                expected1,
                expected2)) {

            fprintf(stderr,
                    "\n随机测试失败\n");

            fprintf(stderr,
                    "测试编号: %d\n",
                    test);

            fprintf(stderr,
                    "size = %d\n",
                    size);

            printf("原数组: ");
            printArray(arr, size);

            fprintf(stderr,
                    "期望结果: %d %d\n",
                    expected1,
                    expected2);

            fprintf(stderr,
                    "实际结果: %d %d\n",
                    actual1,
                    actual2);

            exit(EXIT_FAILURE);
        }
    }
}

int main(void) {

    printf("开始固定测试...\n");

    fixedTests();

    printf("固定测试通过: 6 / 6\n\n");

    printf("开始随机测试...\n");

    randomTests();

    printf("随机测试通过: %d / %d\n",
           RANDOM_TEST_COUNT,
           RANDOM_TEST_COUNT);

    printf("\n"
           "================================\n"
           "全部测试通过\n"
           "================================\n");

    return EXIT_SUCCESS;
}