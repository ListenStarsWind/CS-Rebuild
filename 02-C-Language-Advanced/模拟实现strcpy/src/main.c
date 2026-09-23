#include"my_strcpy.h"
#include <stdio.h>

int main(void){
    char str[] = { "chnwsduihjfgsadygfuyshui" };
	const char* s = "hello word";
    my_strcpy(str, s);
    printf("%s\n", str);
    return 0;    
}
