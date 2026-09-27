#define USE_KMP_STRSTR

#include "my_strstr.h"
#include "stdio.h"
#include "system_info.h"

int main(void) {
    printSystemInfo();
    const char* str1 = "hello word";
    const char* str2 = "llo w";
    printf("%s\n", myStrstr(str1, str2));
    return 0;
}
