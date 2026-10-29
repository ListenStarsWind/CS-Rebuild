#pragma once

namespace wind {
class A {
    // 为静态成员变量的值进行重置
    friend void resetCounters();

    // 读取全局静态变量， 然后对外输出
    friend void out(int& ed, int& ing);

   public:
    A();
    A(const A& object);
    ~A();

   private:
    static int _totalCreatedCount;  // 累计实例化的对象总数，包括已经析构的对象。
    static int _liveObjectCount;    // 当前仍然存活的对象数量
};

void resetCounters();
void out(int& totalCreatedCount, int& liveObjectCount);
void CopyAndDestroy(const A& source);
void info();
}  // namespace wind