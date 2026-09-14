#include <stdio.h>
#include <stdint.h>

static uint64_t power[10][10] = {0};

int is_armstrong_number(uint64_t n){
    if(n < 0)
        return 0;

    uint64_t reset = n;
    int digit_count = 0;
    do{
        digit_count++;
        n = n / 10;
    }while(n);

    n = reset;    
    uint64_t sum = 0;
    for(int i = 0; i < digit_count; i++){
        sum += power[n%10][digit_count];
        n /= 10;
    }

    return sum == reset;
}

int main(void){
    for(int i = 1; i < 10; i++)
    {
        power[i][0] = 1;
        for(int j = 1; j < 10; j++){
            power[i][j] = power[i][j-1]*i;
        }
    }

    for(int i = 0; i < 100000; i++){
        if(is_armstrong_number(i))
            printf("%d ", i);
    }
    printf("\n");
    return 0;    
}
