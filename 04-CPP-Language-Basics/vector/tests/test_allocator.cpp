#define BOOST_TEST_MODULE WindAllocatorTests
#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>

#include "allocator.hpp"

namespace {

struct LifetimeCounter {
    static inline int constructed = 0;
    static inline int destroyed = 0;
    int value;

    explicit LifetimeCounter(int n) : value(n) { ++constructed; }
    ~LifetimeCounter() { ++destroyed; }
};

struct alignas(64) AlignedObject {
    std::uint64_t value = 0;
};

} // namespace

BOOST_AUTO_TEST_CASE(allocator_type_and_rebinding_construction) {
    using IntAlloc = wind::allocator<int>;
    using DoubleAlloc = wind::allocator<double>;

    static_assert(std::is_same_v<IntAlloc::value_type, int>);
    static_assert(std::is_same_v<IntAlloc::size_type, std::size_t>);
    static_assert(std::is_same_v<IntAlloc::difference_type, std::ptrdiff_t>);
    static_assert(std::is_constructible_v<DoubleAlloc, const IntAlloc&>);

    IntAlloc a;
    DoubleAlloc b(a);
    BOOST_CHECK(a == IntAlloc{});
    BOOST_CHECK(a == b);
}

BOOST_AUTO_TEST_CASE(allocate_only_reserves_storage) {
    LifetimeCounter::constructed = 0;
    LifetimeCounter::destroyed = 0;

    wind::allocator<LifetimeCounter> alloc;
    constexpr std::size_t count = 3;
    LifetimeCounter* p = alloc.allocate(count);
    BOOST_REQUIRE(p != nullptr);
    BOOST_TEST(LifetimeCounter::constructed == 0);

    try {
        std::construct_at(p, 10);
        std::construct_at(p + 1, 20);
        BOOST_TEST(LifetimeCounter::constructed == 2);
        BOOST_TEST(p[0].value == 10);
        BOOST_TEST(p[1].value == 20);
        std::destroy_at(p + 1);
        std::destroy_at(p);
    } catch (...) {
        // 上述构造均为不抛异常的简单测试对象；发生异常时先释放原始内存。
        alloc.deallocate(p, count);
        throw;
    }

    BOOST_TEST(LifetimeCounter::destroyed == 2);
    alloc.deallocate(p, count);
    BOOST_TEST(LifetimeCounter::destroyed == 2);
}

BOOST_AUTO_TEST_CASE(overaligned_allocation) {
    wind::allocator<AlignedObject> alloc;
    AlignedObject* p = alloc.allocate(4);
    BOOST_REQUIRE(p != nullptr);

    const auto address = reinterpret_cast<std::uintptr_t>(p);
    BOOST_TEST(address % alignof(AlignedObject) == 0U);
    alloc.deallocate(p, 4);
}

BOOST_AUTO_TEST_CASE(allocation_size_overflow) {
    wind::allocator<int> alloc;
    const std::size_t too_many =
        std::numeric_limits<std::size_t>::max() / sizeof(int) + 1;

    BOOST_CHECK_THROW(alloc.allocate(too_many), std::bad_array_new_length);
}
