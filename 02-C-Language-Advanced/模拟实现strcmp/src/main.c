#include "system_info.h"
#include "my_strcmp.h"
#include <stdio.h>

int main(void){
    printSystemInfo();
    const char* str1 = "abcdef";
	const char* str2 = "abcdfe";
    int ret  = myStrcmp(str1, str2);
    printf("%d\n", ret);
    return 0;    
}
