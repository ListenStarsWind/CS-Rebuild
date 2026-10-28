#include "C.h"

#include <iostream>

namespace wind {
C::C(int i) : _i(i) {
    std::cout << "1" << std::endl;
}

C::C(const C& c) : _i(c._i) {
    std::cout << "2" << std::endl;
}
}  // namespace wind