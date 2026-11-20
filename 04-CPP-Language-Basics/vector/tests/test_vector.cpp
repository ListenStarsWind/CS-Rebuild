#define BOOST_TEST_MODULE wind_vector_public_api
#include <boost/test/unit_test.hpp>

#include "vector.hpp"

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

using Vec = wind::vector<int>;

template <class T, class Alloc>
void check_values(const wind::vector<T, Alloc>& actual,
                  std::initializer_list<T> expected) {
    BOOST_REQUIRE_EQUAL(actual.size(), expected.size());
    std::size_t i = 0;
    for (const auto& value : expected) {
        BOOST_CHECK(actual.data()[i] == value);
        ++i;
    }
}

} // namespace

BOOST_AUTO_TEST_CASE(default_construct_and_observers) {
    Vec v;
    const Vec& cv = v;

    BOOST_CHECK(v.empty());
    BOOST_CHECK_EQUAL(v.size(), 0U);
    BOOST_CHECK_EQUAL(v.capacity(), 0U);
    BOOST_CHECK(cv.empty());
    BOOST_CHECK_EQUAL(cv.size(), 0U);
    BOOST_CHECK_EQUAL(cv.capacity(), 0U);
    BOOST_CHECK_EQUAL(v.data(), nullptr);
    BOOST_CHECK_EQUAL(cv.data(), nullptr);
    BOOST_CHECK(v.begin() == v.end());
    BOOST_CHECK(cv.begin() == cv.end());
}

BOOST_AUTO_TEST_CASE(reserve_resize_clear_and_reuse) {
    Vec v;
    v.reserve(12);
    BOOST_CHECK_GE(v.capacity(), 12U);
    BOOST_CHECK(v.empty());

    const auto reserved_capacity = v.capacity();
    v.resize(4);
    check_values(v, {0, 0, 0, 0});
    BOOST_CHECK_EQUAL(v.capacity(), reserved_capacity);

    v.resize(7, 9);
    check_values(v, {0, 0, 0, 0, 9, 9, 9});
    v.resize(2);
    check_values(v, {0, 0});
    BOOST_CHECK_EQUAL(v.capacity(), reserved_capacity);

    v.clear();
    BOOST_CHECK(v.empty());
    BOOST_CHECK_EQUAL(v.capacity(), reserved_capacity);
    v.push_back(42);
    check_values(v, {42});
    v.resize(v.size());
    check_values(v, {42});
}

BOOST_AUTO_TEST_CASE(push_back_insert_overloads_and_emplace) {
    Vec v;
    const int seven = 7;
    v.push_back(seven);
    v.push_back(8);
    check_values(v, {7, 8});

    auto it = v.insert(v.begin(), 5);
    BOOST_CHECK(it == v.begin());
    check_values(v, {5, 7, 8});

    it = v.insert(v.end(), 2U, 11);
    BOOST_CHECK(it == v.begin() + 3);
    check_values(v, {5, 7, 8, 11, 11});

    it = v.insert(v.begin() + 2, {20, 21});
    BOOST_CHECK(it == v.begin() + 2);
    check_values(v, {5, 7, 20, 21, 8, 11, 11});

    it = v.emplace(v.begin() + 1, 99);
    BOOST_CHECK(it == v.begin() + 1);
    check_values(v, {5, 99, 7, 20, 21, 8, 11, 11});

    const int copy = 33;
    it = v.emplace(v.end(), copy);
    BOOST_CHECK(it == v.end() - 1);
    check_values(v, {5, 99, 7, 20, 21, 8, 11, 11, 33});

    // 插入零个元素应不改变容器。
    const auto old_size = v.size();
    it = v.insert(v.begin() + 1, 0U, 123);
    BOOST_CHECK(it == v.begin() + 1);
    BOOST_CHECK_EQUAL(v.size(), old_size);
}

BOOST_AUTO_TEST_CASE(range_constructor_and_range_insert) {
    const std::vector<int> source{1, 2, 3, 4};
    Vec v(source.begin(), source.end());
    check_values(v, {1, 2, 3, 4});

    const std::vector<int> addition{8, 9};
    auto it = v.insert(v.begin() + 2, addition.begin(), addition.end());
    BOOST_CHECK(it == v.begin() + 2);
    check_values(v, {1, 2, 8, 9, 3, 4});

    // 真正的单趟输入迭代器。
    std::istringstream in("12 13 14");
    Vec from_stream(std::istream_iterator<int>{in}, std::istream_iterator<int>{});
    check_values(from_stream, {12, 13, 14});

    std::istringstream extra("30 31");
    v.insert(v.end(), std::istream_iterator<int>{extra}, std::istream_iterator<int>{});
    check_values(v, {1, 2, 8, 9, 3, 4, 30, 31});
}

BOOST_AUTO_TEST_CASE(self_referential_insert_and_resize) {
    Vec v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    // 强制走扩容路径，检查插入时引用旧存储空间的情形。
    v.insert(v.begin() + 1, 10U, v.data()[2]);
    BOOST_REQUIRE_EQUAL(v.size(), 13U);
    BOOST_CHECK_EQUAL(v.data()[0], 10);
    for (std::size_t i = 1; i != 11; ++i)
        BOOST_CHECK_EQUAL(v.data()[i], 30);
    BOOST_CHECK_EQUAL(v.data()[11], 20);
    BOOST_CHECK_EQUAL(v.data()[12], 30);

    // 先拷贝参数，再进行可能引发扩容的 resize。
    Vec w;
    w.push_back(77);
    w.resize(24, w.data()[0]);
    BOOST_REQUIRE_EQUAL(w.size(), 24U);
    for (std::size_t i = 0; i < w.size(); ++i)
        BOOST_CHECK_EQUAL(w.data()[i], 77);
}

BOOST_AUTO_TEST_CASE(erase_single_range_and_empty_range) {
    Vec v;
    v.insert(v.end(), {1, 2, 3, 4, 5, 6});

    auto it = v.erase(v.begin() + 2);
    BOOST_CHECK(it == v.begin() + 2);
    check_values(v, {1, 2, 4, 5, 6});

    it = v.erase(v.begin() + 1, v.begin() + 3);
    BOOST_CHECK(it == v.begin() + 1);
    check_values(v, {1, 5, 6});

    it = v.erase(v.end(), v.end());
    BOOST_CHECK(it == v.end());
    check_values(v, {1, 5, 6});

    it = v.erase(v.begin(), v.end());
    BOOST_CHECK(it == v.end());
    BOOST_CHECK(v.empty());
}

BOOST_AUTO_TEST_CASE(copy_construct_copy_assign_and_self_assign) {
    Vec a;
    a.insert(a.end(), {3, 4, 5});
    Vec b(a);
    check_values(b, {3, 4, 5});
    BOOST_CHECK(b.data() != a.data());

    b.data()[0] = 99;
    check_values(a, {3, 4, 5});
    check_values(b, {99, 4, 5});

    Vec c;
    c.push_back(-1);
    c = a;
    check_values(c, {3, 4, 5});
    c = c;
    check_values(c, {3, 4, 5});

    Vec empty;
    c = empty;
    BOOST_CHECK(c.empty());
    c = a;
    check_values(c, {3, 4, 5});
}

BOOST_AUTO_TEST_CASE(move_assign_and_swap) {
    Vec source;
    source.insert(source.end(), {10, 20, 30});
    const auto original_data = source.data();

    Vec target;
    target.push_back(999);
    target = std::move(source);
    check_values(target, {10, 20, 30});
    BOOST_CHECK_EQUAL(target.data(), original_data);
    BOOST_CHECK(source.empty());
    BOOST_CHECK_EQUAL(source.data(), nullptr);

    target = std::move(target);
    check_values(target, {10, 20, 30});

    Vec other;
    other.insert(other.end(), {7, 8});
    const auto target_data = target.data();
    const auto other_data = other.data();
    target.swap(other);
    check_values(target, {7, 8});
    check_values(other, {10, 20, 30});
    BOOST_CHECK_EQUAL(target.data(), other_data);
    BOOST_CHECK_EQUAL(other.data(), target_data);
    target.swap(target);
    check_values(target, {7, 8});
}

BOOST_AUTO_TEST_CASE(nontrivial_element_lifetime) {
    wind::vector<std::string> v;
    v.push_back(std::string("one"));
    v.push_back(std::string("two"));
    v.resize(4, std::string("extra"));
    check_values(v, {std::string("one"), std::string("two"), std::string("extra"), std::string("extra")});

    v.erase(v.begin() + 1);
    check_values(v, {std::string("one"), std::string("extra"), std::string("extra")});
    v.clear();
    BOOST_CHECK(v.empty());
}

BOOST_AUTO_TEST_CASE(boundary_error_paths) {
    Vec v;
    v.push_back(1);
    BOOST_CHECK_THROW(v.reserve(static_cast<Vec::size_type>(-1)), std::length_error);
    check_values(v, {1});
}
