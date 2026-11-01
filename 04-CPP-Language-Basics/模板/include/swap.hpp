#pragma once

namespace wind {
// inline void swap(int& here, int& there) {
//     int tmp = here;
//     here = there;
//     there = tmp;
// }

// inline void swap(double& here, double& there) {
//     double tmp = here;
//     here = there;
//     there = tmp;
// }

// inline void swap(char& here, char& there) {
//     char tmp = here;
//     here = there;
//     there = tmp;
// }

template <typename T>
void swap(T& here, T& there) {
    T tmp = here;
    here = there;
    there = tmp;
}
}  // namespace wind