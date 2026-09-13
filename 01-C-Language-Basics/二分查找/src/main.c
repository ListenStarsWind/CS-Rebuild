#include <stdio.h>

int is_greater_or_equal(int a, int x){
    return a >= x;
}

int find_first_x(int* arr, int size,int x){
    int left = 0,mid = 0;
    int right = size - 1;
    while(left < right){
        mid = left + (right - left) / 2;
        if(is_greater_or_equal(arr[mid], x))
        {
            right = mid;
        }
        else {
            left = mid + 1;
        }
    }
    if(arr[left] == x)
        return left;
    else
        return -1;
}

int main(void){
    int arr[10] = {0};
    printf("请输入10个由小到大的数字: ");
    for(int i = 0; i < 10; i++){
        scanf("%d", arr+i);
    }
    int x = -1;
    printf("输入要查询的数字: ");
    scanf("%d", &x);
    int ret  = find_first_x(arr, 10, x);
    if(ret == -1)
        printf("查无此数");
    else
        printf("你所需要的数位于arr[%d]处",ret);
    printf("\n");
    return 0;    
}
