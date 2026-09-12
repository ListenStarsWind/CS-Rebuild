#include <stdio.h>
#include <stdlib.h>

int main(void){
    int i = 0;
    for(i = 1; i <= 9; i++){
        for(int j = 1; j <= i; j++)
            printf("%d*%d=%.2d ",j,i, i*j);
        printf("\n");
    }
    return 0;    
}
