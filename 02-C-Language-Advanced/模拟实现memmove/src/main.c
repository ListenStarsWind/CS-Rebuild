#include "system_info.h"
#include "my_memmove.h"

#include <stdio.h>

int main(void){
    printSystemInfo();
    int arr[10] = {0, 1, 2, 3, 4, 5,6,7, 8,9};
    myMemmove(arr, arr+2, sizeof(int)*5);
    for(int i = 0; i < 10; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
    return 0;    
}
