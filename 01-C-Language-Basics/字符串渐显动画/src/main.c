#include <stdio.h>
#include <unistd.h>

//这段程序实现的是一个“由外向内逐步显现文本”的动画效果：
//初始画面全部由 * 组成，随后每隔 1 秒，将目标字符串中左右对称位置的一对字符显示出来；上一轮已经显示的字符会继续保留。每次更新前清除旧画面，再显示新的状态，最终让 Hello Word 从两侧逐步向中间完整显现出来。

int main(void){
    char arr1[] = {"****hello world****"};
    char arr2[] = {"*******************"};
    int len = sizeof(arr1) / sizeof(arr1[0]);
    
    int left = 0;
    int right = len - 2;
    for(; left <= right; left++,right--){
        arr2[left] = arr1[left];
        arr2[right] = arr1[right];
        printf("%s", arr2);
        fflush(stdout);
        sleep(1);
        printf("\r\033[2K");
    }
    printf("%s\n",arr2);

    return 0;    
}
