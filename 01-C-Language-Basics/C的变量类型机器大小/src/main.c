#include <complex.h>  // 复数类型(C99)
#include <stdbool.h>  // bool
#include <stdio.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // 字符类型
    printf("char                : %zu\n", sizeof(char));
    printf("signed char         : %zu\n", sizeof(signed char));
    printf("unsigned char       : %zu\n", sizeof(unsigned char));

    // 短整型
    printf("short               : %zu\n", sizeof(short));
    printf("unsigned short      : %zu\n", sizeof(unsigned short));

    // 整型
    printf("int                 : %zu\n", sizeof(int));
    printf("unsigned int        : %zu\n", sizeof(unsigned int));

    // 长整型
    printf("long                : %zu\n", sizeof(long));
    printf("unsigned long       : %zu\n", sizeof(unsigned long));

    // 更长整型
    printf("long long           : %zu\n", sizeof(long long));
    printf("unsigned long long  : %zu\n", sizeof(unsigned long long));

    // 浮点类型
    printf("float               : %zu\n", sizeof(float));
    printf("double              : %zu\n", sizeof(double));
    printf("long double         : %zu\n", sizeof(long double));

    // C99 布尔类型
    printf("_Bool               : %zu\n", sizeof(_Bool));

    // void 没有大小，不能 sizeof(void)
    // printf("%zu\n", sizeof(void)); // 错误

    // C99 复数类型
    // float/double/long double 的复数形式
    // 例如：3 + 4i
    printf("float _Complex      : %zu\n", sizeof(float _Complex));
    printf("double _Complex     : %zu\n", sizeof(double _Complex));
    printf("long double Complex: %zu\n", sizeof(long double _Complex));

    return 0;
}