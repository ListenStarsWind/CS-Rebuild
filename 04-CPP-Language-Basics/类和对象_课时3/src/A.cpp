#include "A.h"

namespace wind {
std::ostream& operator<<(std::ostream& out, const A& a) {
    out << "{" << a._i << ", " << a._j << "}";
    return out;
}

A::A(int i, int& j) : _i(i), _j(j) {}

void A::f() {
    _j++;
}
}  // namespace wind