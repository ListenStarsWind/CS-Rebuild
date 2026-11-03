#pragma once

#include <iostream>

namespace wind {
class A {
   public:
    A(int val = 5) : _i(1) {
        _i = val;
    }

    ~A() {
        std::cout << "===" << std::endl;
    }

   private:
    int _i;
};

}  // namespace wind