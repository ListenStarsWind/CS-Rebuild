#include "string_rotation.h"
#include <string.h>
#include <stdio.h>

int main(void){
    char buf1[128] = {0};
    char buf2[128] = {0};
    bool (*pf[3])(const char*, const char*) = {isStringRotationByRepeatedRotation, isStringRotationByConcatenation, isStringRotationByCircularIndex};
    printf("请输入两行字符串: \n");
    while(fgets(buf1, 127, stdin) == NULL);
    while(fgets(buf2, 127, stdin) == NULL);
    int len1 = strlen(buf1);
    int len2 = strlen(buf2);
    if(buf1[len1-1] == '\n')
        buf1[len1-1] = '\0';
    if(buf2[len2-1] == '\n')
        buf2[len2-1] = '\0';
    for(int i = 0; i < 3; i++)
    {
        if(pf[i](buf1, buf2))
            printf("%d: %s", i+1, "它们互为旋转字符串");
        else
            printf("%d: %s", i+1, "它们并非旋转字符串");
        printf("\n");
    }
    return 0;    
}
