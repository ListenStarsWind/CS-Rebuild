#include "B.h"

#include <iostream>

namespace wind {
B::B() : _i(0) {
    std::cout << "构造调用" << std::endl;
}

B::B(const B& obj) : _i(obj._i) {
    std::cout << "拷贝构造" << std::endl;
}

B::~B() {
    std::cout << "析构调用" << std::endl;
}

B& B::operator=(const B& obj) {
    if (&obj != this) {
        _i = obj._i;
    }
    return *this;
}

std::ostream& operator<<(std::ostream& out, const B& obj) {
    out << "_i: " << obj._i;
    return out;
}

void println(B obj) {
    std::cout << obj << std::endl;
}
}  // namespace wind