#include <iostream>
#include <vector>

int main() {
    size_t sz;
    std::vector<int> v;
    sz = v.capacity();
    std::cout << "vector 的内存增长:\n";
    for (int i = 0; i < 100; ++i) {
        v.push_back(i);
        if (sz != v.capacity()) {
            sz = v.capacity();
            std::cout << "容量变化: " << sz << '\n';
        }
    }
    return 0;
}
