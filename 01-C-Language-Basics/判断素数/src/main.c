#include <stdatomic.h>
#include <stdio.h>

#include "prime.h"

void func1(void) {
    for (int i = 2; i <= 100; i++) {
        if (sixKP(i)) printf("%d ", i);
    }
    printf("\n");
}

// 增加了线程安全措施
//
void func2(void) {
    // -1 表示为合数 1 表示为素数
    static int arr[101] = {0};
    // 这是线程共享资源
    static atomic_int flag = 0;
    // 线程栈帧资源相互独立
    // 不同线程使用不同的exp
    // 同一个线程不同时间栈帧不同所以exp还是0
    int expected = 0;
    // 这个函数是原子性的，如果前两个参数相同，flag被置为第三个参数，也就是1
    // 此时别的线程来， 不相同， 就会把自己的exp 置为 flag 的值
    // flag 0 表示主逻辑未进行， 1 表示进行中， 2表示处理完毕
    if (atomic_compare_exchange_strong(&flag, &expected, 1)) {
        for (int i = 2; i <= 100; i++) {
            // 之前已经用合数的因数排除一部分了， 不需要重复
            if (arr[i] == -1) continue;
            // 到这里还未打上标记那就意味着他是素数
            arr[i] = 1;
            for (int j = i * i; j <= 100; j += i) arr[j] = -1;
        }
        // 核心逻辑完成， 置2
        atomic_store(&flag, 2);
    }

    // 没有运行完成在此处空转
    while (atomic_load(&flag) != 2);

    for (int i = 2; i <= 100; i++) {
        if (arr[i] == 1) printf("%d ", i);
    }
    printf("\n");
}

int main(void) {
    func1();
    printf("\n");
    func2();
    return 0;
}
