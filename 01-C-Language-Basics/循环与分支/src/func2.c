#include <stdio.h>
#include "func2.h"

void func2(void){
    int num = 0;
    // CTRL + D 可以输入 EOF
    while(scanf("%d",&num) != EOF){
        while(num--) printf("*");
        printf("\n");
    }
    return ;
}