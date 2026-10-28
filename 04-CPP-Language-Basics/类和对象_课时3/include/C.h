#pragma once

namespace wind {
class C {
   public:
    C(int i);
    C(const C& c);

   private:
    int _i;
};

}  // namespace wind