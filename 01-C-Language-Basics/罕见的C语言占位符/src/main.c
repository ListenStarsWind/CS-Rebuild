#include <stdio.h>

int main(void)
{
    /*
     * 1. 动态宽度 *
     *
     * %*s 中的 * 表示：
     *     字段宽度从参数列表读取
     *
     * 等价于：
     *     printf("%10s", "");
     */
    printf("1. dynamic width:\n");
    printf("[%*s]\n", 10, "");          // 宽度为10，字符串为空，产生10个空格
    printf("\n");


    /*
     * 2. 动态宽度 + 动态精度
     *
     * %*.*s
     *
     * 第一个参数：宽度
     * 第二个参数：精度
     *
     * 等价于：
     *     printf("%10.5s", "Hello World");
     */
    printf("2. dynamic width and precision:\n");
    printf("[%*.*s]\n", 10, 5, "Hello World");
    printf("\n");


    /*
     * 3. 负宽度
     *
     * 负宽度等价于左对齐：
     *
     *     %*s   + width = -10
     *
     * 等价：
     *     %-10s
     */
    printf("3. negative width means left alignment:\n");
    printf("[%*s]\n", -10, "abc");
    printf("\n");


    /*
     * 4. %n
     *
     * %n 不输出内容，
     * 而是把当前已经输出的字符数量写入变量。
     *
     * 注意：
     * 现代代码中几乎不用，
     * 但格式化字符串漏洞分析中经常出现。
     */
    printf("4. %%n write-back:\n");

    int count = 0;

    printf("hello%n world\n", &count);

    printf("characters before %%n: %d\n", count);
    printf("\n");


    /*
     * 5. %p
     *
     * 输出指针地址。
     *
     * 标准要求参数类型为 void *
     */
    printf("5. pointer output:\n");

    int value = 123;

    printf("address of value: %p\n", (void *)&value);
    printf("\n");


    /*
     * 6. %# 形式
     *
     * # 启用替代形式：
     *
     * %#x -> 0x 前缀
     * %#o -> 0 前缀
     */
    printf("6. alternative form:\n");

    printf("hex: %#x\n", 255);
    printf("oct: %#o\n", 255);
    printf("\n");


    /*
     * 7. %.s / %.*s
     *
     * 精度限制字符串最大输出长度。
     *
     * 不修改原字符串。
     */
    printf("7. string precision:\n");

    printf("%.5s\n", "abcdefg");
    printf("%.*s\n", 3, "abcdefg");

    return 0;
}