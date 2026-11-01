#include "add.hpp"

int main() {
    auto i = wind::add(1, 5);
    std::cout << i << std::endl;
    // 如果存在特殊需求, 就是要用模板实例化出的
    // 使用显式实例化即可
    auto j = wind::add<int>(1, 5);
    std::cout << j << std::endl;
    return 0;
}
