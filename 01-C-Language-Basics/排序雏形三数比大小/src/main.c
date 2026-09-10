#include "simVector.h"
#include <stdio.h>

int main(void){
    size_t size = 0;
    printf("输入需要比较的数字个数：");
    while(scanf("%lu",&size)==EOF);
    simV* v = create(size);
    for(size_t i = 0; i < size; i++){
        int num = 0;
        while(scanf("%d", &num) ==  EOF);
        add(num, v);
    }
    int* p = point(v);
    for(size_t i = 0; i < size; i++){
        printf("%d ", p[i]);
    }
    printf("\n");
    destroy(v);
    return 0;    
}
