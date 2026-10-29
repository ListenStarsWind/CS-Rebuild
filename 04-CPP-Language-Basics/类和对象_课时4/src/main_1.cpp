#include <iostream>

#include "A.h"

void test1() {
    std::cout << "1" << std::endl;
    // 一开始, 还没有任何实例化
    // 所以计数都为0
    wind::info();

    wind::A m;
    wind::A n;

    std::cout << "2" << std::endl;
    // 此处这里由通过无参构造进行了两次实例化
    wind::info();
    wind::CopyAndDestroy(wind::A());

    {
        wind::A o;
        std::cout << "5" << std::endl;
        // 匿名对象和CopyAndDestroy中局部对象析构
        // 存在减去2
        wind::info();
    }

    std::cout << "6" << std::endl;
    // 局部变量析构， 存在减去1
    wind::info();

    wind::A p;
    wind::A q;

    std::cout << "7" << std::endl;
    // 再无参构造两次
    wind::info();
}

int main_1() {
    test1();
    std::cout << "8" << std::endl;
    // 没有存在的了
    wind::info();
    return 0;
}
