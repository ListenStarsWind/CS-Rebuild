#include <stdio.h>

int main(void){
    unsigned long long i = 0;
    unsigned long long j = 0;
    while(scanf("%llu",&i) == EOF);
    while(i){
        j = i % 10;
        printf("%llu",j);
        i = i / 10;
    }
    printf("\n");

    return 0;    
}
