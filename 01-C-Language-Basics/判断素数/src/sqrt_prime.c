#include "prime.h"
#include <math.h>

int sqrtP(int n){
    int end = (int)sqrt((double)n);
    for(int i = 2; i <= end; i++){
        if(n % i == 0)
            return 0;
    }
    return 1;
}