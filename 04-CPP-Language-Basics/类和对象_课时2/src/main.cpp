#include "stack.h"

void f(wind::stack t) {
    (void)t;
}

int main() {
    wind::stack s;
    f(s);
    return 0;
}
