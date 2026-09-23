#include "my_strcpy.h"

char *my_strcpy(char *restrict dst, const char *restrict src){
    char* ret = dst;
    while((*dst++=*src++));
    return ret;
}