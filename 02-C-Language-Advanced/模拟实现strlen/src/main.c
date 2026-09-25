#include "system_info.h"
#include "my_strlen.h"
#include <stdio.h>

int main(void){
    printSystemInfo();
    const char* str = "abcdefg";
    printf("%zu\n", myStrlen(str));
    return 0;    
}
