#pragma once

#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace wind {

// 配置器指针萃取
template <class Alloc>
struct allocator_pointer_traits {
    using type = typename Alloc::value_type*;
};

template <class Alloc, class = void>
struct allocator_pointer_traits_selector : allocator_pointer_traits<Alloc> {};

template <class Alloc>
struct allocator_pointer_traits_selector<Alloc, std::void_t<typename Alloc::pointer>> {
    using type = typename Alloc::pointer;
};

// 常指针类型选择
template <class Alloc, class Value, class Pointer, class = void>
struct allocator_const_pointer_selector {
    using type = typename std::pointer_traits<Pointer>::template rebind<const Value>;
};

template <class Alloc, class Value, class Pointer>
struct allocator_const_pointer_selector<Alloc, Value, Pointer,
                                        std::void_t<typename Alloc::const_pointer>> {
    using type = typename Alloc::const_pointer;
};

// 泛型指针类型选择
template <class Alloc, class Pointer, class = void>
struct allocator_void_pointer_selector {
    using type = typename std::pointer_traits<Pointer>::template rebind<void>;
};

template <class Alloc, class Pointer>
struct allocator_void_pointer_selector<Alloc, Pointer, std::void_t<typename Alloc::void_pointer>> {
    using type = typename Alloc::void_pointer;
};

// 常泛型指针类型选择
template <class Alloc, class Pointer, class = void>
struct allocator_const_void_pointer_selector {
    using type = typename std::pointer_traits<Pointer>::template rebind<const void>;
};

template <class Alloc, class Pointer>
struct allocator_const_void_pointer_selector<Alloc, Pointer,
                                             std::void_t<typename Alloc::const_void_pointer>> {
    using type = typename Alloc::const_void_pointer;
};

// 配置器类型重绑定
template <class Alloc, class U>
struct allocator_rebind;

// 提取原配置器模板，并替换第一个类型参数
template <class U, template <class, class...> class Alloc, class T, class... Args>
struct allocator_rebind<Alloc<T, Args...>, U> {
    using type = Alloc<U, Args...>;
};

// 优先使用配置器自身提供的重绑定规则
template <class Alloc, class U, class = void>
struct allocator_rebind_selector : allocator_rebind<Alloc, U> {};

template <class Alloc, class U>
struct allocator_rebind_selector<Alloc, U, std::void_t<typename Alloc::template rebind<U>::other>> {
    using type = typename Alloc::template rebind<U>::other;
};

// 偏移量类型萃取
template <class Alloc, class Pointer, class = void>
struct allocator_difference_type_traits {
    // 为了适应可能存在的自定义指针
    // 先去指针适配器里面把偏移量类型取出来
    using type = typename std::pointer_traits<Pointer>::difference_type;
};

template <class Alloc, class Pointer>
struct allocator_difference_type_traits<Alloc, Pointer,
                                        std::void_t<typename Alloc::difference_type>> {
    // 最后优先选择空间配置器本身就提供的
    using type = typename Alloc::difference_type;
};

// 数量类型萃取
template <class Alloc, class Difference, class = void>
struct allocator_size_type_traits {
    // 数量类型默认使用偏移量的无符号版本
    using type = std::make_unsigned_t<Difference>;
};

template <class Alloc, class Difference>
struct allocator_size_type_traits<Alloc, Difference, std::void_t<typename Alloc::size_type>> {
    using type = typename Alloc::size_type;
};

// 拷贝传播类型萃取
template <class Alloc, class = void>
struct allocator_copy_propagation_traits {
    using type = std::false_type;
};

template <class Alloc>
struct allocator_copy_propagation_traits<
    Alloc, std::void_t<typename Alloc::propagate_on_container_copy_assignment>> {
    using type = typename Alloc::propagate_on_container_copy_assignment;
};

// 移动语义传播类型萃取
template <class Alloc, class = void>
struct allocator_move_propagation_traits {
    using type = std::false_type;
};

template <class Alloc>
struct allocator_move_propagation_traits<
    Alloc, std::void_t<typename Alloc::propagate_on_container_move_assignment>> {
    using type = typename Alloc::propagate_on_container_move_assignment;
};

// 内容交换传播类型萃取
template <class Alloc, class = void>
struct allocator_swap_propagation_traits {
    using type = std::false_type;
};

template <class Alloc>
struct allocator_swap_propagation_traits<Alloc,
                                         std::void_t<typename Alloc::propagate_on_container_swap>> {
    using type = typename Alloc::propagate_on_container_swap;
};

// 恒等性类型萃取
template <class Alloc, class = void>
struct allocator_always_equal_traits {
    // 一个没有非静态成员量的空间配置器显然是恒等的
    using type = std::is_empty<Alloc>::type;
};

template <class Alloc>
struct allocator_always_equal_traits<Alloc, std::void_t<typename Alloc::is_always_equal>> {
    using type = typename Alloc::is_always_equal;
};

template <class Alloc>
struct allocator_traits {
   public:
    // 配置器基本类型
    using allocator_type = Alloc;
    using value_type = typename allocator_type::value_type;

    // 指针类型
    using pointer = typename allocator_pointer_traits_selector<allocator_type>::type;
    using const_pointer =
        typename allocator_const_pointer_selector<allocator_type, value_type, pointer>::type;
    using void_pointer = typename allocator_void_pointer_selector<allocator_type, pointer>::type;
    using const_void_pointer =
        typename allocator_const_void_pointer_selector<allocator_type, pointer>::type;

    // 数量与差值类型
    using difference_type =
        typename allocator_difference_type_traits<allocator_type, pointer>::type;
    using size_type = typename allocator_size_type_traits<allocator_type, difference_type>::type;

    // 描述传播类型特性的类型接口
    using propagate_on_container_copy_assignment =
        typename allocator_copy_propagation_traits<Alloc>::type;
    using propagate_on_container_move_assignment =
        typename allocator_move_propagation_traits<Alloc>::type;
    using propagate_on_container_swap = typename allocator_swap_propagation_traits<Alloc>::type;
    using is_always_equal = typename allocator_always_equal_traits<Alloc>::type;

   public:
    // 配置器类型重绑定
    template <class U>
    using rebind_alloc = typename allocator_rebind_selector<allocator_type, U>::type;

    template <class U>
    using rebind_traits = allocator_traits<rebind_alloc<U>>;

   public:
    // 原始内存管理
    [[nodiscard]]
    static pointer allocate(allocator_type& alloc, size_type n) {
        return alloc.allocate(n);
    }

    static void deallocate(allocator_type& alloc, pointer p, size_type n) {
        alloc.deallocate(p, n);
    }

   public:
    // 对象生命周期管理
    template <class T, class... Args>
    static void construct(Alloc& alloc, T* p, Args&&... args) {
        if constexpr (requires { alloc.construct(p, std::forward<Args>(args)...); }) {
            alloc.construct(p, std::forward<Args>(args)...);
        } else {
            std::construct_at(p, std::forward<Args>(args)...);
        }
    }

    template <class T>
    static void destroy(Alloc& alloc, T* p) {
        if constexpr (requires { alloc.destroy(p); }) {
            alloc.destroy(p);
        } else {
            std::destroy_at(p);
        }
    }

   public:
    // 配置器辅助操作
    static size_type max_size(const allocator_type& alloc) noexcept {
        if constexpr (requires { alloc.max_size(); }) {
            return alloc.max_size();
        } else {
            return std::numeric_limits<size_type>::max() / sizeof(value_type);
        }
    }

    static Alloc select_on_container_copy_construction(const allocator_type& alloc) {
        if constexpr (requires { alloc.select_on_container_copy_construction(); }) {
            return alloc.select_on_container_copy_construction();
        } else {
            return alloc;
        }
    }
};

}  // namespace wind
