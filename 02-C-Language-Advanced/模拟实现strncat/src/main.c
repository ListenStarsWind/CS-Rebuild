#include <stdio.h>
#include <string.h>

#include "my_strncat.h"
#include "system_info.h"

int main(void) {
    printSystemInfo();
    char arr[100] = "xxxxxxxxxxx\0xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    const char* str = "Hope Is the Thing With Feathers";
    size_t n = strlen(str);
    myStrncat(arr, str, n + 3);
    printf("%s\n", arr);
    return 0;
}
