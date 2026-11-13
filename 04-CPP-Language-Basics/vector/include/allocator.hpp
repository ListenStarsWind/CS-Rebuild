#pragma once

#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <new>

namespace wind {

template <class T>
class allocator {
   public:
    using value_type = T;  // 核心数据类型
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using size_type = std::size_t;           // 元素个数类型
    using difference_type = std::ptrdiff_t;  // 元素差值类型
    using byte_size_type = std::size_t;      // 字节数类型

    template <class U>
    struct rebind {
        using other = allocator<U>;
    };

   private:
    static constexpr size_type max_element_count =
        std::numeric_limits<byte_size_type>::max() / sizeof(T);

   public:
    allocator() noexcept = default;

    template <class U>
        requires(!std::same_as<T, U>)
    allocator(const allocator<U>&) noexcept {}

   public:
    [[nodiscard]]
    T* allocate(size_type n) {
        if (n > max_element_count) {
            throw std::bad_array_new_length{};
        }

        const byte_size_type bytes = n * sizeof(T);

        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
            return static_cast<T*>(::operator new(bytes, std::align_val_t{alignof(T)}));
        } else {
            return static_cast<T*>(::operator new(bytes));
        }
    }

    void deallocate(T* p, [[maybe_unused]] size_type n) noexcept {
        // 使用与分配时匹配的释放函数
        if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
            ::operator delete(p, std::align_val_t{alignof(T)});
        } else {
            ::operator delete(p);
        }
    }

    size_type max_size() const noexcept {
        return max_element_count;
    }

    template <class... Args>
    void construct(pointer p, Args&&... args) {
        std::construct_at(p, std::forward<Args>(args)...);
    }

    void destroy(pointer p) {
        std::destroy_at(p);
    }

    allocator<T> select_on_container_copy_construction() const noexcept{
        return *this;
    }

   public:
    template <class U>
    [[nodiscard]]
    constexpr bool operator==(const allocator<U>&) const noexcept {
        return true;
    }
};

}  // namespace wind
