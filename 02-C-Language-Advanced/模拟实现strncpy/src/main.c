#include <stdio.h>
#include <string.h>

#include "my_strncpy.h"
#include "system_info.h"

int main(void) {
    printSystemInfo();
    char arr[50] = "xxxxxxxxxxxxxxxxxxxxxxx";
    const char* str = "hello word";
    size_t n = strlen(str);
    myStrncpy(arr, str, n + 3);
    printf("%s\n", arr);
    return 0;
}
