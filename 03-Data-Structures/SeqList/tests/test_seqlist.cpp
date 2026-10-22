#define BOOST_TEST_MODULE SeqListTests
#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <limits>
#include <memory>
#include <vector>

extern "C" {
#include "SeqList.h"
}

namespace {
struct SeqListDeleter {
    void operator()(SeqList* p) const noexcept {
        if (p) SL_Destroy(p);
    }
};
using SeqListPtr = std::unique_ptr<SeqList, SeqListDeleter>;

SeqListPtr make_list() { return SeqListPtr(SL_Create()); }

void check_values(const SeqList* sl, const std::vector<SLDataType>& expected) {
    BOOST_REQUIRE_EQUAL(SL_Size(sl), expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        SLDataType actual = 0;
        BOOST_REQUIRE_MESSAGE(SL_Get(sl, i, &actual) == 0,
                              "SL_Get failed at index " << i);
        BOOST_CHECK_EQUAL(actual, expected[i]);
    }
}
} // namespace

BOOST_AUTO_TEST_SUITE(seqlist)

BOOST_AUTO_TEST_CASE(create_and_clear) {
    auto sl = make_list();
    BOOST_REQUIRE(sl != nullptr);
    BOOST_CHECK_EQUAL(SL_Size(sl.get()), 0U);
    BOOST_CHECK_EQUAL(SL_IsEmpty(sl.get()), 1);
    BOOST_CHECK_LE(SL_Size(sl.get()), SL_Capacity(sl.get()));

    BOOST_REQUIRE_EQUAL(SL_PushBack(sl.get(), 42), 0);
    const auto capacity = SL_Capacity(sl.get());
    SL_Clear(sl.get());
    BOOST_CHECK_EQUAL(SL_Size(sl.get()), 0U);
    BOOST_CHECK_EQUAL(SL_IsEmpty(sl.get()), 1);
    BOOST_CHECK_EQUAL(SL_Capacity(sl.get()), capacity);
    SL_Clear(sl.get());
    BOOST_CHECK_EQUAL(SL_Size(sl.get()), 0U);
}

BOOST_AUTO_TEST_CASE(reserve_semantics) {
    auto sl = make_list();
    BOOST_REQUIRE(sl != nullptr);
    BOOST_REQUIRE_EQUAL(SL_PushBack(sl.get(), 123), 0);
    const auto old_capacity = SL_Capacity(sl.get());
    BOOST_REQUIRE_EQUAL(SL_Reserve(sl.get(), old_capacity), 0);
    BOOST_CHECK_EQUAL(SL_Capacity(sl.get()), old_capacity);
    BOOST_REQUIRE_EQUAL(SL_Reserve(sl.get(), 0), 0);
    BOOST_CHECK_EQUAL(SL_Capacity(sl.get()), old_capacity);

    // No assumption about the automatic growth factor.
    const std::size_t target = old_capacity + 20;
    BOOST_REQUIRE_EQUAL(SL_Reserve(sl.get(), target), 0);
    BOOST_CHECK_GE(SL_Capacity(sl.get()), target);
    check_values(sl.get(), {123});
}

BOOST_AUTO_TEST_CASE(push_back_and_growth) {
    auto sl = make_list();
    BOOST_REQUIRE(sl != nullptr);
    constexpr int count = 128;
    for (int i = 0; i < count; ++i) {
        BOOST_REQUIRE_EQUAL(SL_PushBack(sl.get(), i * 3 - 10), 0);
        BOOST_CHECK_EQUAL(SL_Size(sl.get()), static_cast<std::size_t>(i + 1));
        BOOST_CHECK_GE(SL_Capacity(sl.get()), SL_Size(sl.get()));
    }
    std::vector<SLDataType> expected;
    for (int i = 0; i < count; ++i) expected.push_back(i * 3 - 10);
    check_values(sl.get(), expected);
}

BOOST_AUTO_TEST_CASE(insert_overlap_and_boundary) {
    auto sl = make_list();
    BOOST_REQUIRE(sl != nullptr);
    BOOST_REQUIRE_EQUAL(SL_Insert(sl.get(), 0, 20), 0); // empty, pos == size
    BOOST_REQUIRE_EQUAL(SL_Insert(sl.get(), 0, 10), 0); // front shift
    BOOST_REQUIRE_EQUAL(SL_Insert(sl.get(), 2, 40), 0); // tail
    BOOST_REQUIRE_EQUAL(SL_Insert(sl.get(), 2, 30), 0); // middle shift
    check_values(sl.get(), {10, 20, 30, 40});

    const auto before_size = SL_Size(sl.get());
    const auto before_capacity = SL_Capacity(sl.get());
    BOOST_CHECK_NE(SL_Insert(sl.get(), before_size + 1, 99), 0);
    BOOST_CHECK_EQUAL(SL_Size(sl.get()), before_size);
    BOOST_CHECK_EQUAL(SL_Capacity(sl.get()), before_capacity);
    check_values(sl.get(), {10, 20, 30, 40});
}

BOOST_AUTO_TEST_CASE(push_front_and_pop) {
    auto sl = make_list();
    BOOST_REQUIRE(sl != nullptr);
    BOOST_CHECK_NE(SL_PopFront(sl.get()), 0);
    BOOST_CHECK_NE(SL_PopBack(sl.get()), 0);
    BOOST_REQUIRE_EQUAL(SL_PushFront(sl.get(), 30), 0);
    BOOST_REQUIRE_EQUAL(SL_PushFront(sl.get(), 20), 0);
    BOOST_REQUIRE_EQUAL(SL_PushFront(sl.get(), 10), 0);
    check_values(sl.get(), {10, 20, 30});

    BOOST_REQUIRE_EQUAL(SL_PopFront(sl.get()), 0);
    check_values(sl.get(), {20, 30});
    BOOST_REQUIRE_EQUAL(SL_PopBack(sl.get()), 0);
    check_values(sl.get(), {20});
    BOOST_REQUIRE_EQUAL(SL_PopBack(sl.get()), 0);
    BOOST_CHECK_EQUAL(SL_IsEmpty(sl.get()), 1);
    BOOST_CHECK_NE(SL_PopBack(sl.get()), 0);
}

BOOST_AUTO_TEST_CASE(erase_and_invalid_indices) {
    auto sl = make_list();
    BOOST_REQUIRE(sl != nullptr);
    for (int i = 0; i < 5; ++i) BOOST_REQUIRE_EQUAL(SL_PushBack(sl.get(), i + 1), 0);
    BOOST_REQUIRE_EQUAL(SL_Erase(sl.get(), 2), 0);
    check_values(sl.get(), {1, 2, 4, 5});
    BOOST_REQUIRE_EQUAL(SL_Erase(sl.get(), 0), 0);
    BOOST_REQUIRE_EQUAL(SL_Erase(sl.get(), SL_Size(sl.get()) - 1), 0);
    check_values(sl.get(), {2, 4});
    const auto size = SL_Size(sl.get());
    BOOST_CHECK_NE(SL_Erase(sl.get(), size), 0);
    BOOST_CHECK_EQUAL(SL_Size(sl.get()), size);
    check_values(sl.get(), {2, 4});
}

BOOST_AUTO_TEST_CASE(find_get_set) {
    auto sl = make_list();
    BOOST_REQUIRE(sl != nullptr);
    for (int x : {8, 3, 8, 5}) BOOST_REQUIRE_EQUAL(SL_PushBack(sl.get(), x), 0);

    std::size_t pos = std::numeric_limits<std::size_t>::max();
    BOOST_REQUIRE_EQUAL(SL_Find(sl.get(), 8, &pos), 0);
    BOOST_CHECK_EQUAL(pos, 0U); // first match
    pos = 77;
    BOOST_CHECK_NE(SL_Find(sl.get(), 999, &pos), 0);
    BOOST_CHECK_EQUAL(pos, 77U); // failure leaves output unchanged

    SLDataType out = -100;
    BOOST_CHECK_NE(SL_Get(sl.get(), SL_Size(sl.get()), &out), 0);
    BOOST_CHECK_EQUAL(out, -100); // invalid lookup doesn't overwrite output
    BOOST_CHECK_NE(SL_Set(sl.get(), SL_Size(sl.get()), 100), 0);
    check_values(sl.get(), {8, 3, 8, 5});
    BOOST_REQUIRE_EQUAL(SL_Set(sl.get(), 1, 42), 0);
    check_values(sl.get(), {8, 42, 8, 5});
}

BOOST_AUTO_TEST_CASE(large_reserve_failure_preserves_state) {
    auto sl = make_list();
    BOOST_REQUIRE(sl != nullptr);
    BOOST_REQUIRE_EQUAL(SL_PushBack(sl.get(), 11), 0);
    BOOST_REQUIRE_EQUAL(SL_PushBack(sl.get(), 22), 0);
    const auto size = SL_Size(sl.get());
    const auto capacity = SL_Capacity(sl.get());

    // This must not be passed to realloc after an unchecked multiplication.
    BOOST_CHECK_NE(SL_Reserve(sl.get(), std::numeric_limits<std::size_t>::max()), 0);
    BOOST_CHECK_EQUAL(SL_Size(sl.get()), size);
    BOOST_CHECK_EQUAL(SL_Capacity(sl.get()), capacity);
    check_values(sl.get(), {11, 22});
}

BOOST_AUTO_TEST_SUITE_END()
