#pragma once
#include <ostream>

namespace wind {
class B {
    friend std::ostream& operator<<(std::ostream& out, const B& obj);

   public:
    B();
    B(const B& obj);
    ~B();
    B& operator=(const B& obj);

   private:
    int _i;
};

void println(B obj);
}  // namespace wind