#include "SeqList.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned long checks = 0;
static unsigned long failures = 0;

#define CHECK(expr) do { \
    ++checks; \
    if (!(expr)) { \
        ++failures; \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
    } \
} while (0)

#define REQUIRE(expr) do { \
    ++checks; \
    if (!(expr)) { \
        ++failures; \
        fprintf(stderr, "FAIL %s:%d: %s (aborting case)\n", __FILE__, __LINE__, #expr); \
        goto cleanup; \
    } \
} while (0)

static void check_contents(const_pSeqList sl, const int *expected, size_t n)
{
    CHECK(SL_Size(sl) == n);
    CHECK(SL_Capacity(sl) >= SL_Size(sl));
    CHECK(SL_IsEmpty(sl) == (n == 0));
    size_t actual = SL_Size(sl);
    if (actual != n) return;
    for (size_t i = 0; i < n; ++i) {
        int value = 0;
        int rc = SL_Get(sl, i, &value);
        CHECK(rc == 0);
        if (rc == 0) CHECK(value == expected[i]);
    }
}

static void test_creation_and_capacity(void)
{
    puts("[CASE] creation and capacity");
    pSeqList sl = SL_Create();
    REQUIRE(sl != NULL);
    CHECK(SL_Size(sl) == 0);
    CHECK(SL_IsEmpty(sl) == 1);
    CHECK(SL_Capacity(sl) >= SL_Size(sl));
    REQUIRE(SL_Reserve(sl, 8) == 0);
    CHECK(SL_Capacity(sl) >= 8);
    size_t cap = SL_Capacity(sl);
    CHECK(SL_Reserve(sl, 3) == 0);
    CHECK(SL_Capacity(sl) == cap);
    CHECK(SL_Reserve(sl, cap) == 0);
    CHECK(SL_Capacity(sl) == cap);
    CHECK(SL_Size(sl) == 0);
cleanup:
    if (sl) SL_Destroy(sl);
}

static void test_push_and_insert(void)
{
    puts("[CASE] push and insert");
    pSeqList sl = SL_Create();
    REQUIRE(sl != NULL);
    REQUIRE(SL_PushBack(sl, 10) == 0);
    REQUIRE(SL_PushBack(sl, 30) == 0);
    REQUIRE(SL_PushFront(sl, 5) == 0);
    REQUIRE(SL_Insert(sl, 2, 20) == 0);
    REQUIRE(SL_Insert(sl, SL_Size(sl), 40) == 0);
    REQUIRE(SL_Insert(sl, 0, -1) == 0);
    const int expected[] = {-1, 5, 10, 20, 30, 40};
    check_contents(sl, expected, sizeof(expected) / sizeof(expected[0]));
    size_t before_size = SL_Size(sl);
    size_t before_cap = SL_Capacity(sl);
    CHECK(SL_Insert(sl, before_size + 1, 99) != 0);
    check_contents(sl, expected, sizeof(expected) / sizeof(expected[0]));
    CHECK(SL_Capacity(sl) == before_cap);
cleanup:
    if (sl) SL_Destroy(sl);
}

static void test_delete_and_bounds(void)
{
    puts("[CASE] delete and bounds");
    pSeqList sl = SL_Create();
    REQUIRE(sl != NULL);
    CHECK(SL_PopBack(sl) != 0);
    CHECK(SL_PopFront(sl) != 0);
    CHECK(SL_Erase(sl, 0) != 0);
    for (int i = 0; i < 6; ++i) REQUIRE(SL_PushBack(sl, i) == 0);
    REQUIRE(SL_Erase(sl, 2) == 0);
    const int e1[] = {0, 1, 3, 4, 5};
    check_contents(sl, e1, 5);
    REQUIRE(SL_PopFront(sl) == 0);
    REQUIRE(SL_PopBack(sl) == 0);
    const int e2[] = {1, 3, 4};
    check_contents(sl, e2, 3);
    CHECK(SL_Erase(sl, SL_Size(sl)) != 0);
    CHECK(SL_Set(sl, SL_Size(sl), 999) != 0);
    int out = 123456;
    CHECK(SL_Get(sl, SL_Size(sl), &out) != 0);
    check_contents(sl, e2, 3);
cleanup:
    if (sl) SL_Destroy(sl);
}

static void test_find_get_set_clear(void)
{
    puts("[CASE] find, get, set, clear");
    pSeqList sl = SL_Create();
    REQUIRE(sl != NULL);
    size_t pos = 777;
    CHECK(SL_Find(sl, 42, &pos) != 0);
    CHECK(pos == 777);
    REQUIRE(SL_PushBack(sl, 8) == 0);
    REQUIRE(SL_PushBack(sl, 3) == 0);
    REQUIRE(SL_PushBack(sl, 8) == 0);
    REQUIRE(SL_Find(sl, 8, &pos) == 0);
    CHECK(pos == 0);
    pos = 777;
    CHECK(SL_Find(sl, 99, &pos) != 0);
    CHECK(pos == 777);
    REQUIRE(SL_Set(sl, 1, -123) == 0);
    const int expected[] = {8, -123, 8};
    check_contents(sl, expected, 3);
    size_t cap = SL_Capacity(sl);
    SL_Clear(sl);
    CHECK(SL_Size(sl) == 0);
    CHECK(SL_IsEmpty(sl) == 1);
    CHECK(SL_Capacity(sl) == cap);
    REQUIRE(SL_PushBack(sl, INT_MIN) == 0);
    REQUIRE(SL_PushBack(sl, INT_MAX) == 0);
    const int extremes[] = {INT_MIN, INT_MAX};
    check_contents(sl, extremes, 2);
cleanup:
    if (sl) SL_Destroy(sl);
}

static void test_stress(void)
{
    puts("[CASE] repeated shifting and growth");
    enum { N = 300 };
    pSeqList sl = SL_Create();
    REQUIRE(sl != NULL);
    for (int i = 0; i < N; ++i) REQUIRE(SL_PushFront(sl, i) == 0);
    CHECK(SL_Size(sl) == N);
    for (int i = 0; i < N; ++i) {
        int v = 0;
        REQUIRE(SL_Get(sl, (size_t)i, &v) == 0);
        CHECK(v == N - 1 - i);
    }
    for (int i = 0; i < N; ++i) REQUIRE(SL_PopFront(sl) == 0);
    CHECK(SL_Size(sl) == 0);
    CHECK(SL_IsEmpty(sl) == 1);
    for (int i = 0; i < N; ++i) REQUIRE(SL_PushBack(sl, i) == 0);
    for (int i = N - 1; i >= 0; --i) {
        int v = 0;
        REQUIRE(SL_Get(sl, (size_t)i, &v) == 0);
        CHECK(v == i);
        REQUIRE(SL_PopBack(sl) == 0);
    }
    CHECK(SL_Size(sl) == 0);
cleanup:
    if (sl) SL_Destroy(sl);
}

static void test_failed_reserve_preserves_data(void)
{
    puts("[CASE] impossible reserve preserves state");
    pSeqList sl = SL_Create();
    REQUIRE(sl != NULL);
    REQUIRE(SL_PushBack(sl, 11) == 0);
    REQUIRE(SL_PushBack(sl, 22) == 0);
    REQUIRE(SL_PushBack(sl, 33) == 0);
    size_t original_cap = SL_Capacity(sl);
    /* SIZE_MAX elements cannot fit in a valid allocation of int. */
    int rc = SL_Reserve(sl, SIZE_MAX);
    CHECK(rc != 0);
    CHECK(SL_Capacity(sl) == original_cap);
    const int expected[] = {11, 22, 33};
    check_contents(sl, expected, 3);
cleanup:
    if (sl) SL_Destroy(sl);
}

int main(void)
{
    test_creation_and_capacity();
    test_push_and_insert();
    test_delete_and_bounds();
    test_find_get_set_clear();
    test_stress();
    test_failed_reserve_preserves_data();
    printf("\nChecks: %lu, failures: %lu\n", checks, failures);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


