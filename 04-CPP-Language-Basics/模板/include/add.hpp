#pragma once

#include <iostream>

namespace wind {

// 引用底层有开销, 起码有个指针
// 所以直接就用 int 了
// 但这并不是主要的注意点
inline int add(int v1, int v2) {
    std::cout << "int" << std::endl;
    return v1 + v2;
}

// 模板不清楚具体类型, 所以干脆常引用算了
// 如果是自定义类, 没有实现加法是他自己的事
template <typename T>
T add(const T& v1, const T& v2) {
    std::cout << "template" << std::endl;
    return v1 + v2;
}
}  // namespace wind
