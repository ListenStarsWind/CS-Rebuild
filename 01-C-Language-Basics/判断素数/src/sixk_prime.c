#include "prime.h"

int sixKP(int n){
    if(n == 2 || n == 3)
        return 1;
    int i = n % 6;
    if(i == 1 || i == 5)
        return sqrtP(n);
    else
        return 0;
}