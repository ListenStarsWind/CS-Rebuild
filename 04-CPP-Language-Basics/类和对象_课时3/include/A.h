#pragma once
#include <iostream>

namespace wind {
std::istream& operator>>(std::istream& in, int& i);

class A {
    friend std::ostream& operator<<(std::ostream& out, const A& a);
    friend std::istream& operator>>(std::ostream& in, A& a) = delete;

   public:
    A(int i, int& j);
    void f();

   private:
    const int _i;
    int& _j;
};
}  // namespace wind