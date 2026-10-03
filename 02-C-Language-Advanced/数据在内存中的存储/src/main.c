#ifdef _WIN32
#include <windows.h>
#endif

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// int main(void) {
// #ifdef _WIN32
//     SetConsoleOutputCP(CP_UTF8);
// #endif

//     /* 编译期：查看编译器提供的目标信息。 */
// #if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)

// #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
//     printf("编译期判断：小端\n");
// #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
//     printf("编译期判断：大端\n");
// #else
//     printf("编译期判断：其他字节序\n");
// #endif

// #elif defined(_MSC_VER) && \
//     (defined(_M_IX86) || defined(_M_X64) || defined(_M_ARM64) || defined(_M_ARM64EC))

//     printf("编译期判断：小端（依据 Windows 目标架构）\n");

// #else
//     printf("编译期判断：没有可用的判断信息\n");
// #endif

//     /* 运行期：实际读取这个整数的各个字节。 */
//     uint32_t value = 0x12345678;
//     const unsigned char* p = (const unsigned char*)&value;

//     printf("从低地址到高地址：\n");

//     for (size_t i = 0; i < sizeof value; ++i) {
//         printf("%p : %02X\n", (const void*)(p + i), (unsigned int)p[i]);
//     }

//     if (p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12) {
//         printf("实际观察：小端\n");
//     } else if (p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78) {
//         printf("实际观察：大端\n");
//     } else {
//         printf("实际观察：其他字节排列\n");
//     }

//     return 0;
// }

// // 实验一
// int main(void) {
//     char a = -1;
//     signed char b = -1;
//     unsigned char c = -1;
//     printf("a=%d,b=%d,c=%d\n", a, b, c);
//     return 0;
// }

//// 实验二
// int main(void) {
//     char a = -128;
//     printf("%u\n", (unsigned int)a);
//     return 0;
// }

//// 实验三
// int main(void){
//     char a = 128;
//     printf("%u\n", (unsigned int)a);
//     return 0;
// }

// // 实验四
// int main(void) {
//     char a[1000];
//     int i;
//     for (i = 0; i < 1000; i++) {
//         a[i] = -1 - i;
//     }
//     printf("%zu\n", strlen(a));
//     return 0;
// }

// 实验七
int main(void) {
    uint32_t a[4] = {1, 2, 3, 4};
    const unsigned char* p = (const unsigned char*)a;
    uint32_t value;

    memcpy(&value, p + 1, sizeof value);

    printf("%" PRIx32 ",%" PRIx32 "\n", a[3], value);
    return 0;
}
