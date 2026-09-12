#include <stdio.h>

int main(void){
    int i = 0;
    int j = 0;
    int k = 0;
    while(scanf("%d",&i)!=1);
    while(scanf("%d",&j)!=1);
    while((k = i % j)){
        i = j;
        j = k;
    }
    printf("二者最大公约数为%d\n", j);
    return 0;    
}
