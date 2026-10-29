#include "A.h"

#include <iostream>

namespace wind {
// 在C++11及其之前， 类中全局变量
// 的定义及初始化放在类外
int A::_totalCreatedCount = 0;
int A::_liveObjectCount = 0;

A::A() {
    _totalCreatedCount++;
    _liveObjectCount++;
}

A::A(const A& object) {
    (void)object;
    _totalCreatedCount++;
    _liveObjectCount++;
}

A::~A() {
    _liveObjectCount--;
}

void resetCounters() {
    A::_totalCreatedCount = 0;
    A::_liveObjectCount = 0;
}

// 使用这种输出型参数， 没有别的特殊意思
// 只是想少写一个函数--用两个函数分别返回
// 也是完全没有问题的
void out(int& totalCreatedCount, int& liveObjectCount) {
    totalCreatedCount = A::_totalCreatedCount;
    liveObjectCount = A::_liveObjectCount;
}

void CopyAndDestroy(const A& source) {
    // 常引用暂时延长了匿名对象的生命周期
    // 双计数加1
    std::cout << "3" << std::endl;
    info();

    A tmp = source;

    std::cout << "4" << std::endl;
    // 使用拷贝构造再加1
    info();

    (void)tmp;
}

void info() {
    static int totalCreatedCount;
    static int liveObjectCount;
    wind::out(totalCreatedCount, liveObjectCount);
    std::cout << "已经实例化的对象个数：" << totalCreatedCount << std::endl;
    std::cout << "目前还存在的对象个数：" << liveObjectCount << std::endl;
}
}  // namespace wind