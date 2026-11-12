#define BOOST_TEST_MODULE WindAllocatorTraitsTests
#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

#include "allocator.hpp"
#include "allocator_traits.hpp"

namespace {

template <class T>
struct MinimalAllocator {
    using value_type = T;

    T* allocate(std::size_t n) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }
    void deallocate(T* p, std::size_t) noexcept { ::operator delete(p); }
};

template <class T>
struct FancyPointer {
    using element_type = T;
    using difference_type = std::int32_t;
    template <class U>
    using rebind = FancyPointer<U>;
};

template <class T>
struct FancyAllocator {
    using value_type = T;
    using pointer = FancyPointer<T>;
};

struct ResourceState {
    int constructions = 0;
    int destructions = 0;
    int copy_selects = 0;
};

template <class T>
struct CustomAllocator {
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using difference_type = std::int32_t;
    using size_type = std::uint32_t;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::false_type;

    ResourceState* state = nullptr;
    CustomAllocator() = default;
    explicit CustomAllocator(ResourceState* s) : state(s) {}
    template <class U>
    CustomAllocator(const CustomAllocator<U>& other) : state(other.state) {}

    template <class U>
    struct rebind {
        using other = CustomAllocator<U>;
    };

    T* allocate(size_type n) {
        return static_cast<T*>(::operator new(static_cast<std::size_t>(n) * sizeof(T)));
    }
    void deallocate(T* p, size_type) noexcept { ::operator delete(p); }

    template <class U, class... Args>
    void construct(U* p, Args&&... args) {
        ++state->constructions;
        std::construct_at(p, std::forward<Args>(args)...);
    }
    template <class U>
    void destroy(U* p) {
        ++state->destructions;
        std::destroy_at(p);
    }
    size_type max_size() const noexcept { return 123; }
    CustomAllocator select_on_container_copy_construction() const {
        ++state->copy_selects;
        return *this;
    }
};

// 第二个参数是非类型模板参数，不能使用 Alloc<T, Args...> 的默认重绑定。
template <class T, std::size_t Id>
struct TaggedAllocator {
    using value_type = T;
    using pointer = T*;
    template <class U>
    struct rebind {
        using other = TaggedAllocator<U, Id>;
    };
};

struct Tracked {
    static inline int alive = 0;
    int value;
    explicit Tracked(int n) : value(n) { ++alive; }
    ~Tracked() { --alive; }
};

} // namespace

BOOST_AUTO_TEST_CASE(default_associated_types) {
    using Traits = wind::allocator_traits<MinimalAllocator<int>>;

    static_assert(std::is_same_v<Traits::value_type, int>);
    static_assert(std::is_same_v<Traits::pointer, int*>);
    static_assert(std::is_same_v<Traits::const_pointer, const int*>);
    static_assert(std::is_same_v<Traits::void_pointer, void*>);
    static_assert(std::is_same_v<Traits::const_void_pointer, const void*>);
    static_assert(std::is_same_v<Traits::difference_type, std::ptrdiff_t>);
    static_assert(std::is_same_v<Traits::size_type, std::make_unsigned_t<std::ptrdiff_t>>);
    static_assert(!Traits::propagate_on_container_copy_assignment::value);
    static_assert(!Traits::propagate_on_container_move_assignment::value);
    static_assert(!Traits::propagate_on_container_swap::value);
    static_assert(Traits::is_always_equal::value);
    BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(fancy_pointer_type_rebinding) {
    using Traits = wind::allocator_traits<FancyAllocator<int>>;
    static_assert(std::is_same_v<Traits::pointer, FancyPointer<int>>);
    static_assert(std::is_same_v<Traits::const_pointer, FancyPointer<const int>>);
    static_assert(std::is_same_v<Traits::void_pointer, FancyPointer<void>>);
    static_assert(std::is_same_v<Traits::const_void_pointer, FancyPointer<const void>>);
    static_assert(std::is_same_v<Traits::difference_type, std::int32_t>);
    static_assert(std::is_same_v<Traits::size_type, std::uint32_t>);
    BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(rebinding_allocator_and_traits) {
    using Traits = wind::allocator_traits<wind::allocator<int>>;
    static_assert(std::is_same_v<Traits::rebind_alloc<double>, wind::allocator<double>>);
    static_assert(std::is_same_v<
        Traits::rebind_traits<double>,
        wind::allocator_traits<wind::allocator<double>>>);

    using TaggedTraits = wind::allocator_traits<TaggedAllocator<int, 7>>;
    static_assert(std::is_same_v<
        TaggedTraits::rebind_alloc<double>, TaggedAllocator<double, 7>>);
    BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(custom_associated_types_and_policies) {
    using Traits = wind::allocator_traits<CustomAllocator<int>>;
    static_assert(std::is_same_v<Traits::size_type, std::uint32_t>);
    static_assert(std::is_same_v<Traits::difference_type, std::int32_t>);
    static_assert(Traits::propagate_on_container_copy_assignment::value);
    static_assert(Traits::propagate_on_container_move_assignment::value);
    static_assert(Traits::propagate_on_container_swap::value);
    static_assert(!Traits::is_always_equal::value);
    BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(default_allocate_construct_destroy) {
    using Alloc = MinimalAllocator<Tracked>;
    using Traits = wind::allocator_traits<Alloc>;
    Alloc alloc;
    Tracked::alive = 0;

    Tracked* p = Traits::allocate(alloc, 2);
    BOOST_REQUIRE(p != nullptr);
    BOOST_TEST(Tracked::alive == 0);
    Traits::construct(alloc, p, 42);
    BOOST_TEST(Tracked::alive == 1);
    BOOST_TEST(p->value == 42);
    Traits::destroy(alloc, p);
    BOOST_TEST(Tracked::alive == 0);
    Traits::deallocate(alloc, p, 2);
}

BOOST_AUTO_TEST_CASE(custom_operation_priority) {
    using Alloc = CustomAllocator<Tracked>;
    using Traits = wind::allocator_traits<Alloc>;
    ResourceState state;
    Alloc alloc(&state);
    Tracked::alive = 0;

    Tracked* p = Traits::allocate(alloc, 1);
    BOOST_REQUIRE(p != nullptr);
    Traits::construct(alloc, p, 17);
    BOOST_TEST(state.constructions == 1);
    BOOST_TEST(Tracked::alive == 1);
    Traits::destroy(alloc, p);
    BOOST_TEST(state.destructions == 1);
    BOOST_TEST(Tracked::alive == 0);
    Traits::deallocate(alloc, p, 1);

    BOOST_TEST(Traits::max_size(alloc) == 123U);
    auto selected = Traits::select_on_container_copy_construction(alloc);
    BOOST_TEST(state.copy_selects == 1);
    BOOST_TEST(selected.state == &state);
}

BOOST_AUTO_TEST_CASE(default_helper_operations) {
    using Alloc = MinimalAllocator<int>;
    using Traits = wind::allocator_traits<Alloc>;
    Alloc alloc;
    BOOST_TEST(Traits::max_size(alloc) > 0U);
    auto copied = Traits::select_on_container_copy_construction(alloc);
    static_assert(std::is_same_v<decltype(copied), Alloc>);
}
