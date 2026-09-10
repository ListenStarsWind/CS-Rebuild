#include <stdio.h>

int _func1(int n)
{
    if(n == 1)
        return 1;

    return 10 + 10 * _func1(n-1);
}

int _func2(int n){
    if(n == 1)
        return 1;

    return 10 * _func2(n-1);
}

int main(void){
    int n = 0;
    printf("输入位数：");
    while(scanf("%d", &n) !=  1);
    int tmp = 0;
    printf("输入要查询的数字(0~9)");
    while(scanf("%d", &tmp) !=  1);
    printf("其中一共有%d个位数为%d\n",tmp == 0 ? _func2(n) : _func1(n), tmp);
    return 0;    
}
