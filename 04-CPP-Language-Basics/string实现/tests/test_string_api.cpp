#define BOOST_TEST_MODULE WindStringApiTests
#include <boost/test/unit_test.hpp>

#include "string.hpp"

#include <algorithm>
#include <compare>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {
using String = wind::string;

void expect_bytes(const String& actual, std::string_view expected) {
    BOOST_REQUIRE_EQUAL(actual.size(), expected.size());
    const std::string got(actual.data(), actual.size());
    BOOST_CHECK_EQUAL(got, std::string(expected));
    BOOST_CHECK_EQUAL(actual.data()[actual.size()], '\0');
}

std::string snapshot(const String& s) {
    return std::string(s.data(), s.size());
}

// 比较插入操作的结果时，先复制原内容，避免参考实现发生源区间重叠。
void check_self_insert(std::string_view initial, std::size_t pos,
                       std::size_t source_pos, std::size_t count) {
    std::string expected(initial);
    String actual(expected.c_str());
    const std::string source = expected.substr(source_pos, count);
    expected.insert(pos, source);

    actual.insert(pos, actual.data() + source_pos, count);
    expect_bytes(actual, expected);
}
} // namespace

BOOST_AUTO_TEST_SUITE(construction_and_storage)

BOOST_AUTO_TEST_CASE(default_and_c_string_constructor) {
    String empty;
    expect_bytes(empty, "");
    BOOST_CHECK_GE(empty.capacity(), empty.size());

    String short_text("Hello");
    expect_bytes(short_text, "Hello");

    String long_text("abcdefghijklmnopqrstuvwxyz0123456789");
    expect_bytes(long_text, "abcdefghijklmnopqrstuvwxyz0123456789");
    BOOST_CHECK_GE(long_text.capacity(), long_text.size());
}

BOOST_AUTO_TEST_CASE(range_constructor_with_forward_and_input_iterators) {
    const std::string expected("a\0bc\n", 5);
    String by_range(expected.begin(), expected.end());
    expect_bytes(by_range, expected);

    std::istringstream in("one two\nthree");
    String by_input{std::istreambuf_iterator<char>(in),
                    std::istreambuf_iterator<char>()};
    expect_bytes(by_input, "one two\nthree");
}

BOOST_AUTO_TEST_CASE(copy_constructor_and_assignment_are_independent) {
    String source("abcdefghijklmnopqrstuvwxyz0123456789");
    String copied(source);
    expect_bytes(copied, snapshot(source));
    copied[0] = 'Z';
    BOOST_CHECK_EQUAL(source[0], 'a');

    String assigned("short");
    assigned = source;
    expect_bytes(assigned, snapshot(source));
    assigned[1] = 'Y';
    BOOST_CHECK_EQUAL(source[1], 'b');

    assigned = assigned;
    BOOST_CHECK_EQUAL(assigned[1], 'Y');
    expect_bytes(source, "abcdefghijklmnopqrstuvwxyz0123456789");
}

BOOST_AUTO_TEST_CASE(clear_reset_and_capacity_management) {
    String s("hello");
    s.reserve(100);
    BOOST_CHECK_GE(s.capacity(), 100u);
    expect_bytes(s, "hello");

    const auto before_clear = s.capacity();
    s.clear();
    expect_bytes(s, "");
    BOOST_CHECK_EQUAL(s.capacity(), before_clear);

    s.append("abc");
    s.shrink_to_fit();
    expect_bytes(s, "abc");
    BOOST_CHECK_GE(s.capacity(), s.size());
    BOOST_CHECK_LE(s.capacity(), before_clear);

    s.reset_storage();
    expect_bytes(s, "");
    BOOST_CHECK_GE(s.capacity(), s.size());
}

BOOST_AUTO_TEST_CASE(sso_boundary_and_heap_transition) {
    String s("123456789012345"); // 通常为 15 字符的 SSO 边界
    expect_bytes(s, "123456789012345");
    s.push_back('6');
    expect_bytes(s, "1234567890123456");
    s.shrink_to_fit();
    expect_bytes(s, "1234567890123456");
    s.resize(3);
    s.shrink_to_fit();
    expect_bytes(s, "123");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(element_access_and_iterators)

BOOST_AUTO_TEST_CASE(index_at_and_const_access) {
    String s("abc");
    s[1] = 'X';
    BOOST_CHECK_EQUAL(s.at(1), 'X');
    const String& cs = s;
    BOOST_CHECK_EQUAL(cs[0], 'a');
    BOOST_CHECK_EQUAL(cs.at(2), 'c');
    BOOST_CHECK_THROW(s.at(3), std::out_of_range);
    BOOST_CHECK_THROW(cs.at(3), std::out_of_range);
    expect_bytes(s, "aXc");
}

BOOST_AUTO_TEST_CASE(iterator_ranges_and_mutable_data) {
    String s("abc");
    BOOST_CHECK_EQUAL(s.end() - s.begin(), 3);
    BOOST_CHECK_EQUAL(s.cend() - s.cbegin(), 3);
    const String& cs = s;
    BOOST_CHECK_EQUAL(cs.end() - cs.begin(), 3);
    BOOST_CHECK_EQUAL(std::string(s.begin(), s.end()), "abc");
    BOOST_CHECK_EQUAL(std::string(cs.begin(), cs.end()), "abc");

    *s.begin() = 'A';
    s.data()[1] = 'B';
    expect_bytes(s, "ABc");
    BOOST_CHECK(s.begin() == s.data());
    BOOST_CHECK(s.end() == s.data() + s.size());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(append_and_concatenation)

BOOST_AUTO_TEST_CASE(all_basic_append_overloads) {
    String s("a");
    s.append("bc");
    s.append("deXYZ", 2);
    s.append(3, '!');
    s.append({'1', '2'});
    expect_bytes(s, "abcde!!!12");

    String other("012345");
    s.append(other, 2, 3);
    s.append(other);
    expect_bytes(s, "abcde!!!12234012345");
}

BOOST_AUTO_TEST_CASE(append_can_preserve_embedded_nuls) {
    String s;
    const char raw[] = {'A', '\0', 'B', '\n', '\0'};
    s.append(raw, sizeof(raw));
    expect_bytes(s, std::string_view(raw, sizeof(raw)));
}

BOOST_AUTO_TEST_CASE(append_from_input_iterator_range) {
    std::istringstream in("a b\nc");
    String s("start:");
    s.append(std::istreambuf_iterator<char>(in),
             std::istreambuf_iterator<char>());
    expect_bytes(s, "start:a b\nc");
}

BOOST_AUTO_TEST_CASE(append_string_view_and_slice) {
    String s("(");
    const std::string_view view("ABCDEFG");
    s.append(view);
    s.append(view, 2, 3);
    expect_bytes(s, "(ABCDEFGCDE");
}

BOOST_AUTO_TEST_CASE(append_self_reference_with_and_without_growth) {
    String short_s("abc");
    short_s.append(short_s);
    expect_bytes(short_s, "abcabc");

    String long_s("abcdefghijklmnop");
    long_s.append(long_s.data() + 3, 8);
    expect_bytes(long_s, "abcdefghijklmnopdefghijk");

    String repeated("abcdefghijklmnopqrstuvwxyz");
    repeated.append(repeated);
    expect_bytes(repeated, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
}

BOOST_AUTO_TEST_CASE(plus_equals_overloads_and_push_back) {
    String s("A");
    String x("BC");
    s += x;
    s += "DE";
    s += 'F';
    s += {'G', 'H'};
    s.push_back('I');
    expect_bytes(s, "ABCDEFGHI");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(insert_and_erase)

BOOST_AUTO_TEST_CASE(index_insert_overloads) {
    String s("ace");
    s.insert(1, "b");
    s.insert(3, "d", 1);
    s.insert(5, 2, '!');
    expect_bytes(s, "abcde!!");

    const String extra("1234");
    s.insert(0, extra, 1, 2);
    s.insert(s.size(), extra);
    expect_bytes(s, "23abcde!!1234");
}

BOOST_AUTO_TEST_CASE(insert_string_view_overloads) {
    String s("AB");
    std::string_view source("123456");
    s.insert(1, source);
    s.insert(1, source, 2, 2);
    expect_bytes(s, "A34123456B");
}

BOOST_AUTO_TEST_CASE(insert_self_reference_before_after_and_crossing) {
    check_self_insert("abcdefgh", 6, 1, 3); // 源区间完全位于插入点前
    check_self_insert("abcdefgh", 2, 4, 3); // 源区间完全位于插入点后
    check_self_insert("abcdefgh", 4, 2, 4); // 源区间跨越插入点
    check_self_insert("abcdefghijklmnop", 3, 1, 12); // 触发扩容
}

BOOST_AUTO_TEST_CASE(iterator_insert_overloads) {
    String s("ac");
    auto pos = s.insert(s.cbegin() + 1, 'b');
    BOOST_CHECK(pos == s.begin() + 1);
    expect_bytes(s, "abc");

    pos = s.insert(s.cbegin() + 3, 2, '!');
    BOOST_CHECK(pos == s.begin() + 3);
    expect_bytes(s, "abc!!");

    pos = s.insert(s.cbegin() + 1, {'x', 'y'});
    BOOST_CHECK(pos == s.begin() + 1);
    expect_bytes(s, "axybc!!");

    std::istringstream in("PQ");
    pos = s.insert(s.cend(), std::istreambuf_iterator<char>(in),
                   std::istreambuf_iterator<char>());
    BOOST_CHECK(pos == s.end() - 2);
    expect_bytes(s, "axybc!!PQ");
}

BOOST_AUTO_TEST_CASE(erase_overloads_and_bounds) {
    String s("0123456789");
    s.erase(2, 3);
    expect_bytes(s, "0156789");

    auto pos = s.erase(s.cbegin() + 2);
    BOOST_CHECK(pos == s.begin() + 2);
    expect_bytes(s, "016789");

    pos = s.erase(s.cbegin() + 1, s.cbegin() + 3);
    BOOST_CHECK(pos == s.begin() + 1);
    expect_bytes(s, "0789");

    BOOST_CHECK_THROW(s.erase(s.size() + 1), std::out_of_range);
    s.erase();
    expect_bytes(s, "");
}

BOOST_AUTO_TEST_CASE(insert_bounds) {
    String s("abc");
    BOOST_CHECK_THROW(s.insert(4, "X"), std::out_of_range);
    BOOST_CHECK_THROW(s.insert(4, 1, 'x'), std::out_of_range);
    BOOST_CHECK_THROW(s.insert(1, String("xyz"), 4, 1), std::out_of_range);
    expect_bytes(s, "abc");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(resize_swap_and_search)

BOOST_AUTO_TEST_CASE(resize_shrink_grow_and_default_nul) {
    String s("abc");
    s.resize(2);
    expect_bytes(s, "ab");
    s.resize(5, 'x');
    expect_bytes(s, "abxxx");
    s.resize(7);
    expect_bytes(s, std::string_view("abxxx\0\0", 7));
}

BOOST_AUTO_TEST_CASE(swap_sso_heap_and_heap_heap) {
    String short_s("cat");
    String long_s("abcdefghijklmnopqrstuvwxyz0123456789");
    short_s.swap(long_s);
    expect_bytes(short_s, "abcdefghijklmnopqrstuvwxyz0123456789");
    expect_bytes(long_s, "cat");

    String another("1234567890abcdefghijklmnopqrstuvwxyz");
    short_s.swap(another);
    expect_bytes(short_s, "1234567890abcdefghijklmnopqrstuvwxyz");
    expect_bytes(another, "abcdefghijklmnopqrstuvwxyz0123456789");
}

BOOST_AUTO_TEST_CASE(find_overloads_and_search_positions) {
    String s("abcabc");
    BOOST_CHECK_EQUAL(s.find(String("abc")), 0u);
    BOOST_CHECK_EQUAL(s.find(String("abc"), 1), 3u);
    BOOST_CHECK_EQUAL(s.find("bc"), 1u);
    BOOST_CHECK_EQUAL(s.find("bc", 2), 4u);
    BOOST_CHECK_EQUAL(s.find('c', 3), 5u);
    BOOST_CHECK_EQUAL(s.find("not-there"), String::npos);
    BOOST_CHECK_EQUAL(s.find("", s.size()), s.size());
    BOOST_CHECK_EQUAL(s.find("", s.size() + 1), String::npos);

    const std::string_view view("cab");
    BOOST_CHECK_EQUAL(s.find(view), 2u);
}

BOOST_AUTO_TEST_CASE(find_binary_patterns) {
    const char raw[] = {'a', '\0', 'b', '\0', 'c'};
    String s;
    s.append(raw, sizeof(raw));
    const char pattern[] = {'\0', 'b'};
    BOOST_CHECK_EQUAL(s.find(pattern, 0, 2), 1u);
    BOOST_CHECK_EQUAL(s.find('\0'), 1u);
    BOOST_CHECK_EQUAL(s.find('\0', 2), 3u);
}

BOOST_AUTO_TEST_CASE(substr_and_bounds) {
    String s("abcdef");
    expect_bytes(s.substr(), "abcdef");
    expect_bytes(s.substr(2), "cdef");
    expect_bytes(s.substr(2, 2), "cd");
    expect_bytes(s.substr(s.size()), "");
    BOOST_CHECK_THROW(s.substr(s.size() + 1), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(comparisons)

BOOST_AUTO_TEST_CASE(equality_and_three_way_ordering) {
    const String a("abc"), b("abd"), c("abc"), prefix("ab");
    BOOST_CHECK(a == c);
    BOOST_CHECK(!(a == b));
    BOOST_CHECK(a != b);
    BOOST_CHECK(a < b);
    BOOST_CHECK(b > a);
    BOOST_CHECK(a <= c);
    BOOST_CHECK(c >= a);
    BOOST_CHECK(prefix < a);
    BOOST_CHECK((a <=> b) < 0);
    BOOST_CHECK((a <=> c) == 0);
    BOOST_CHECK((b <=> a) > 0);
    BOOST_CHECK("abc" == a); // C++20 反向比较候选
    BOOST_CHECK("abb" < a);  // C++20 反向三路比较候选
}

BOOST_AUTO_TEST_CASE(compare_embedded_nul_bytes) {
    String a, b;
    const char left[]  = {'a', '\0', 'b'};
    const char right[] = {'a', '\0', 'c'};
    a.append(left, sizeof(left));
    b.append(right, sizeof(right));
    BOOST_CHECK(a < b);
    BOOST_CHECK(a != b);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(randomized_differential)

BOOST_AUTO_TEST_CASE(compare_many_operations_with_std_string) {
    // 固定种子使测试可复现；包含空格和内嵌 NUL，并频繁跨越 SSO 边界。
    std::mt19937 rng(20261109u);
    auto pick = [&](std::size_t maximum) {
        return std::uniform_int_distribution<std::size_t>(0, maximum)(rng);
    };

    String actual;
    std::string expected;
    constexpr char alphabet[] = {'a', 'Z', ' ', '\0', '\n', '7'};

    for (int iteration = 0; iteration < 500; ++iteration) {
        std::size_t op = pick(3);
        if (expected.size() > 120 && op < 2)
            op = 2; // 避免测试数据无限增长

        BOOST_TEST_CONTEXT("iteration=" << iteration << ", op=" << op) {
            if (op == 0 || op == 1) {
                std::string chunk;
                const auto count = pick(20);
                for (std::size_t i = 0; i < count; ++i)
                    chunk.push_back(alphabet[pick(sizeof(alphabet) - 1)]);

                if (op == 0) {
                    actual.append(chunk.data(), chunk.size());
                    expected.append(chunk.data(), chunk.size());
                } else {
                    const auto pos = pick(expected.size());
                    actual.insert(pos, chunk.data(), chunk.size());
                    expected.insert(pos, chunk.data(), chunk.size());
                }
            } else if (op == 2) {
                const auto pos = pick(expected.size());
                const auto count = pick(25);
                actual.erase(pos, count);
                expected.erase(pos, count);
            } else {
                const auto new_size = pick(65);
                const char fill = alphabet[pick(sizeof(alphabet) - 1)];
                actual.resize(new_size, fill);
                expected.resize(new_size, fill);
            }
            expect_bytes(actual, expected);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
