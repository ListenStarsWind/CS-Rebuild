#include "func1.h"
#include <stdio.h>

void func1(void){
    int num = 0;
    while(scanf("%d",&num) == 1){
        if(num >= 42){
            const char* s1 = "我不知道该写什么";
            printf("%s\n",s1);
        }else {
            printf("%s\n", "我要跳出这个循环");
            break;
        }
    }
}