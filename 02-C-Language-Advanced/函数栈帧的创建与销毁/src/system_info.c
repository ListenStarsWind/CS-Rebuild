#include "system_info.h"

#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>

void printSystemInfo(void)
{
    printf("\n");
    printf("==================================================\n");
    printf(" 运行环境信息\n");
    printf("==================================================\n");

    printf(" sizeof(char)       : %zu byte\n", sizeof(char));
    printf(" sizeof(short)      : %zu byte\n", sizeof(short));
    printf(" sizeof(int)        : %zu byte\n", sizeof(int));
    printf(" sizeof(long)       : %zu byte\n", sizeof(long));
    printf(" sizeof(long long)  : %zu byte\n", sizeof(long long));
    printf(" sizeof(size_t)     : %zu byte\n", sizeof(size_t));
    printf(" sizeof(void*)      : %zu byte\n", sizeof(void*));
    printf(" sizeof(time_t)     : %zu byte\n", sizeof(time_t));

    printf("\n");

    printf(" CHAR_BIT           : %d\n", CHAR_BIT);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    printf(" 字节序             : Little Endian\n");
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    printf(" 字节序             : Big Endian\n");
#else
    printf(" 字节序             : Unknown\n");
#endif

#if __STDC_VERSION__
    printf(" C 标准版本         : %ld\n", __STDC_VERSION__);
#endif

#ifdef __clang__
    printf(" 编译器             : Clang\n");
#endif

#ifdef __GNUC__
    printf(" GCC 兼容           : Yes\n");
#endif

#ifdef _WIN32
    printf(" 操作系统           : Windows\n");
#elif __linux__
    printf(" 操作系统           : Linux\n");
#elif __APPLE__
    printf(" 操作系统           : macOS\n");
#else
    printf(" 操作系统           : Unknown\n");
#endif

    printf("==================================================\n");
}
