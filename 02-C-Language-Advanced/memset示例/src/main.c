#include "system_info.h"
#include <stdio.h>
#include <string.h>

int main(void){
    char str[] = {"hello world"};
    memset(str, 'x', 7);
    printf("%s\n", str);
    printSystemInfo();
    return 0;    
}
