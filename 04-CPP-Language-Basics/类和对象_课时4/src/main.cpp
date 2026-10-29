#include "B.h"
#include <iostream>

void test2(){
    wind::B x;
    wind::println(x);
    std::cout<<"=================="<<std::endl;
    wind::println(wind::B());
    std::cout<<"=================="<<std::endl;
    auto y = wind::B();
    std::cout<<"=================="<<std::endl;
}

wind::B test3(){
    wind::B x;
    return x;
}

int main(){
    auto y = test3();
    return 0;
}