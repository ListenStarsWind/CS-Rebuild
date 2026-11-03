#include <iostream>
#include <string>

int main() {
    std::string url("https://legacy.cplusplus.com/reference/string/string/find/");
    size_t n = url.find(':');
    if (n != std::string::npos) {
        auto scheme = url.substr(0, n);
        std::cout << "协议种类: " << scheme << std::endl;
    } else {
        std::cerr << "url 存在错误" << std::endl;
    }

    // 第二个参数规定了查找的起始位置, 缺省值为0
    size_t m = url.find('/', n + 3);
    if (m != std::string::npos) {
        auto hostname = url.substr(n + 3, m - (n + 3));
        std::cout << "域名: " << hostname << std::endl;
    } else {
        std::cerr << "url 存在错误" << std::endl;
    }

    n = url.find('/', m + 1);
    if (n != std::string::npos) {
        auto path = url.substr(m + 1);
        std::cout << "资源路径: " << path << std::endl;
    } else {
        std::cerr << "url 存在错误" << std::endl;
    }

    return 0;
}
