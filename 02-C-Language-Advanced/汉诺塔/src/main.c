//汉诺塔是递归中的经典问题，其玩法是：给定3根柱子A, B, C，初始有n个圆环（记为1至n号，由小到大序）在A上，最终目标是通过若干步移动，将所有圆环都移动到B上，且移动的规则为：
//①1次移动只能1个圆环（且应当是A / B / C最上面的圆环）
//②始终要求小号圆环堆叠在大号圆环上（形如金字塔形，例如不能将2号圆环移动到1号圆环之上）

#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

uint64_t towers(int n){
    if(n <= 0)
        return 0;
    if(n == 1)
        return 1;
    return 2*towers(n-1) + 1;
}


int main(void){
    int n = 0;
    scanf("%d", &n);
    printf("%" PRIu64 "\n", towers(n));
    return 0;    
}
