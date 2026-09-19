#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_rotation.h"


#define RANDOM_TEST_COUNT 10000
#define MAX_STRING_LENGTH 100


typedef bool (*RotationFunction)(const char*, const char*);


typedef struct {
    const char* name;
    RotationFunction function;
} RotationMethod;


typedef struct {
    const char* str1;
    const char* str2;
    bool expected;
} FixedTestCase;


/* ================================================================
 * 三种待测试方法
 * ================================================================ */

static const RotationMethod METHODS[] = {
    {
        "RepeatedRotation",
        isStringRotationByRepeatedRotation
    },
    {
        "Concatenation",
        isStringRotationByConcatenation
    },
    {
        "CircularIndex",
        isStringRotationByCircularIndex
    }
};


static const int METHOD_COUNT =
    sizeof(METHODS) / sizeof(METHODS[0]);



/* ================================================================
 * 固定测试
 * ================================================================ */

static const FixedTestCase FIXED_TESTS[] = {
    {"abcde",  "cdeab",  true},
    {"abcde",  "abced",  false},
    {"abcde",  "abcde",  true},

    /* 重复字符，真正旋转起点不是第一处同字符 */
    {"abac",   "acab",   true},

    /* 字符数量相同，但不是旋转关系 */
    {"aabc",   "abac",   false},

    {"aaab",   "abaa",   true},

    /* 长度不同 */
    {"abcde",  "cdeabx", false},

    /* 大小写、数字、标点 */
    {"Ab1,c",  "1,cAb",  true},

    /* 大小写敏感 */
    {"abc",    "Bca",    false},

    /* 空字符串 */
    {"",       "",       true}
};


static const int FIXED_TEST_COUNT =
    sizeof(FIXED_TESTS) / sizeof(FIXED_TESTS[0]);



/* ================================================================
 * 打印失败信息
 * ================================================================ */

static void printFailure(
    const char* method,
    const char* str1,
    const char* str2,
    bool expected,
    bool actual
)
{
    printf("\n================================\n");
    printf("测试失败\n");
    printf("方法: %s\n", method);
    printf("str1: \"%s\"\n", str1);
    printf("str2: \"%s\"\n", str2);
    printf("期望: %s\n", expected ? "true" : "false");
    printf("实际: %s\n", actual ? "true" : "false");
    printf("================================\n");
}



/* ================================================================
 * 检查一组输入
 * ================================================================ */

static bool checkCase(
    const char* str1,
    const char* str2,
    bool expected
)
{
    for (int i = 0; i < METHOD_COUNT; ++i) {

        bool actual =
            METHODS[i].function(str1, str2);


        if (actual != expected) {

            printFailure(
                METHODS[i].name,
                str1,
                str2,
                expected,
                actual
            );

            return false;
        }
    }


    return true;
}



/* ================================================================
 * 构造旋转字符串
 * ================================================================ */

static void makeRotation(
    const char* source,
    char* target,
    int size,
    int shift
)
{
    for (int i = 0; i < size; ++i) {

        target[i] =
            source[(i + shift) % size];
    }


    target[size] = '\0';
}



/* ================================================================
 * 随机字符串
 * ================================================================ */

static void makeRandomString(
    char* str,
    int size
)
{
    static const char CHARSET[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        " ,.-_!?";


    const int charsetSize =
        (int)strlen(CHARSET);


    for (int i = 0; i < size; ++i) {

        str[i] =
            CHARSET[rand() % charsetSize];
    }


    str[size] = '\0';
}



/* ================================================================
 * main
 * ================================================================ */

int main(void)
{
    /* 固定种子，方便失败时复现 */
    srand(20260919);


    /* ------------------------------------------------------------
     * 固定测试
     * ------------------------------------------------------------ */

    for (int i = 0; i < FIXED_TEST_COUNT; ++i) {

        if (!checkCase(
                FIXED_TESTS[i].str1,
                FIXED_TESTS[i].str2,
                FIXED_TESTS[i].expected
            )) {

            return EXIT_FAILURE;
        }
    }


    printf(
        "固定测试通过: %d 组\n",
        FIXED_TEST_COUNT
    );



    /* ------------------------------------------------------------
     * 随机测试
     *
     * 每轮生成：
     *
     * 1. 一组确定具有旋转关系的数据
     * 2. 一组确定不具有旋转关系的数据
     *
     * 所以每轮实际检查两种情况。
     * ------------------------------------------------------------ */

    for (
        int test = 1;
        test <= RANDOM_TEST_COUNT;
        ++test
    ) {

        int size =
            rand() % MAX_STRING_LENGTH + 1;


        char source[MAX_STRING_LENGTH + 1];
        char rotated[MAX_STRING_LENGTH + 1];
        char invalid[MAX_STRING_LENGTH + 1];


        makeRandomString(
            source,
            size
        );


        int shift =
            rand() % size;


        /* ------------------------
         * 正例
         * ------------------------ */

        makeRotation(
            source,
            rotated,
            size,
            shift
        );


        if (!checkCase(
                source,
                rotated,
                true
            )) {

            printf(
                "随机测试编号: %d\n",
                test
            );

            return EXIT_FAILURE;
        }



        /* ------------------------
         * 反例
         *
         * 修改一个字符。
         *
         * 因为字符计数发生变化，
         * 所以一定不可能再是旋转关系。
         * ------------------------ */

        strcpy(
            invalid,
            rotated
        );


        int position =
            rand() % size;


        char oldChar =
            invalid[position];


        do {

            invalid[position] =
                (char)('!' + rand() % 94);

        } while (
            invalid[position] == oldChar
        );


        if (!checkCase(
                source,
                invalid,
                false
            )) {

            printf(
                "随机测试编号: %d\n",
                test
            );

            return EXIT_FAILURE;
        }



        if (test % 1000 == 0) {

            printf(
                "已通过 %d 组随机测试\n",
                test
            );
        }
    }



    printf("\n");
    printf("================================\n");
    printf("字符串旋转关系测试完成\n");
    printf("固定测试: %d 组全部通过\n", FIXED_TEST_COUNT);
    printf("随机测试: %d 组全部通过\n", RANDOM_TEST_COUNT);
    printf("三种实现结果全部一致\n");
    printf("================================\n");


    return EXIT_SUCCESS;
}
