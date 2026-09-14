#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

uint64_t ways[100] = {0, 1, 2};

void init_stair_ways(void)
{
    for(int i = 3; i < 100; i++)
        ways[i] = ways[i-1] + ways[i-2];
}

uint64_t get_stair_ways(int stairs)
{
    if(stairs < 0 || stairs >= 100)
        return 0;

    return ways[stairs];
}

int main(void){
    int arr[10] = {0};
    init_stair_ways();
    for(int i = 0; i < 10; i++)
        scanf("%d", arr + i);
    for(int i = 0; i < 10; i++)
        printf("%" PRIu64 " ", get_stair_ways(arr[i]));
    printf("\n");
    return 0;    
}
