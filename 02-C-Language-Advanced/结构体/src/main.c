#include <stdio.h>

/*
 * 位段的具体布局并没有被 C 标准完全固定下来。
 *
 * 尤其值得观察的一点是：
 *
 *     相邻位段在声明过程中突然改变基础类型时，
 *     编译器 / ABI 应该如何处理当前已经开始使用的存储单元？
 *
 * 不同平台对此可以采用不同规则，因此同一份源码最终可能得到
 * 不同的结构体大小和内存布局。
 */
struct Bits {
    /*
     * a 以 unsigned short 作为基础类型。
     *
     * 接下来的 b 突然改成 unsigned int，
     * 然后 c 又重新改回 unsigned short。
     *
     * 真正要观察的就是这两个“类型切换点”：
     *
     *     unsigned short -> unsigned int
     *     unsigned int   -> unsigned short
     *
     * 不同 ABI 对相邻位段能否继续共享当前存储区域、
     * 是否需要结束当前分配单元并重新开始等问题，
     * 可以采取不同的处理方式。
     *
     * 例如 Microsoft ABI 对相邻位段共享 allocation unit
     * 有比较明确的限制：只有基础整数类型大小相同，并且下一个位段
     * 仍能完整放入当前分配单元时，才会继续打包到同一个单元中。
     *
     * System V AMD64 ABI 的位段布局规则则不同，因此这组连续改变
     * 基础类型的位段可能形成与 Windows 不同的最终布局。
     *
     * 这也是为什么位段不适合脱离具体 ABI 去推断精确内存表示。
     */
    unsigned short a : 4;
    unsigned int b : 4;
    unsigned short c : 4;
};

/*
 * 再把 Bits 放入普通结构体中，只是为了让内部布局差异更容易观察。
 *
 * 如果不同 ABI 得到的 sizeof(struct Bits) 不同，
 * 那么这个差异还会继续影响外层结构体中后续成员的位置，
 * 最终表现为 sizeof(struct Test) 也不同。
 *
 * 这里的根源仍然是上面位段基础类型切换时的布局规则差异，
 * 外层结构体本身没有使用任何特殊对齐手段。
 */
struct Test {
    char head;
    struct Bits bits;
    char tail;
};

int main(void) {
    /*
     * 位段不能直接使用 offsetof，也不能对位段成员取地址，
     * 因此这里直接通过 sizeof 观察最终布局结果。
     */
    printf("sizeof(struct Bits) = %zu\n", sizeof(struct Bits));
    printf("sizeof(struct Test) = %zu\n", sizeof(struct Test));

    return 0;
}