#pragma once

#include <algorithm>
#include <type_traits>

#include "allocator.hpp"
#include "allocator_traits.hpp"

namespace wind {
template <class T, class Alloc = wind::allocator<T>>
    requires(std::is_object_v<T>)  // 约束: 这应该是个对象类型, 而不是引用, 函数什么其他类型
class vector {
   public:
    using value_type = T;
    using allocator_type = Alloc;

   private:
    using allocator_traits_type = wind::allocator_traits<allocator_type>;

    static_assert(std::is_same_v<value_type, typename allocator_traits_type::value_type>,
                  "wind::vector: allocator_traits value_type must match T");

   public:
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename allocator_traits_type::pointer;
    using const_pointer = typename allocator_traits_type::const_pointer;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using difference_type = typename allocator_traits_type::difference_type;
    using size_type = typename allocator_traits_type::size_type;

   private:
    pointer _start;         // 指向开始元素
    pointer _finish;        // 指向下一个元素应该存放的位置
    pointer _endofstorage;  // 指向所分配存储空间的末尾
    allocator_type _alloc;  // 空间配置器;

   public:
    vector()
        : _start(nullptr), _finish(nullptr), _endofstorage(nullptr), _alloc(allocator_type()) {};

    size_type size() noexcept {
        return _finish - _start;
    }

    size_type capacity() noexcept {
        return _endofstorage - _start;
    }

    bool empty() noexcept {
        return _start == _finish;
    }

    iterator begin() noexcept {
        return _start;
    }

    iterator end() noexcept {
        return _finish;
    }

    pointer data() noexcept {
        return _start;
    }

    void reserve(size_type minCapacity) {
        // 取出适配器提供的最大数目, 作为检查的判断依据
        const size_type maxCapacity = allocator_traits_type::max_size(_alloc);

        if (minCapacity > maxCapacity) throw std::length_error("wind::vector::reserve");

        size_type oldSize = size();
        size_type oldCapacity = capacity();

        if (oldCapacity >= minCapacity) return;

        // 小于等于1时, 为了避免 / 2 仍旧为0, 预选容量定为4
        size_type increment = oldCapacity <= 1 ? 4 : oldCapacity / 2;
        // 对当前容量进行审议, 在确保加上0.5不超过上限情况下才加上0.5
        // 但 maxCapacity 极小时, 4 可能会大于它, 造成 increment 大于上限的情况
        // 所以 increment 不能参与加减计算
        size_type newCapacity =
            increment >= maxCapacity - oldCapacity ? maxCapacity : oldCapacity + increment;

        newCapacity = std::max(newCapacity, minCapacity);
        pointer ptr = allocator_traits_type::allocate(_alloc, newCapacity);
        pointer curr = ptr;

        size_type idx = 0;
        try {
            for (; idx < oldSize; idx++) {
                // 使用原来的对象参与新对象的构造
                // std::move_if_noexcept: 如果移动语义构造保证不抛异常, 则转为右值, 匹配移动语义构造
                // 移动构造可能抛异常且允许拷贝时，优先使用拷贝构造,不支持拷贝构造时，仍然尝试移动
                allocator_traits_type::construct(_alloc, curr + idx,
                                                 std::move_if_noexcept(_start[idx]));
            }
        } catch (...) {
            // 如果中途拷贝构造抛出异常, 析构之前已经成功构造的新元素, 并释放新空间
            size_type firefighter = 0;
            for (; firefighter < idx; firefighter++) {
                allocator_traits_type::destroy(_alloc, curr + firefighter);
            }
            allocator_traits_type::deallocate(_alloc, ptr, newCapacity);
            // 向上交接异常
            throw;
        }

        // 为了能够进行回退, 我们必须在新元素全部构造完成之后, 再析构原先对应的旧元素
        for (idx = 0; idx < oldSize; idx++) {
            allocator_traits_type::destroy(_alloc, _start + idx);
        }
        // 释放原先的旧空间(在他们确实存在的情况下)
        if (_start != nullptr) {
            allocator_traits_type::deallocate(_alloc, _start, oldCapacity);
        }

        // 三指针进行状态更新
        _start = ptr;
        _finish = ptr + oldSize;
        _endofstorage = _start + newCapacity;
        return;
    }

    iterator insert(const_iterator pos, size_type count, const_reference value) {
        if (count == 0) return pos;

        size_type oldSize = size();
        difference_type offset = pos - _start;
        if (offset < 0 || offset > oldSize)
            throw std::out_of_range("wind::vector::insert: invalid iterator position");
        reserve(oldSize + count);

        pointer dest = _finish;
        pointer src = _start + offset;
        difference_type idx = count - 1;
        try {
            for (; idx >= 0; --idx) {
                allocator_traits_type::construct(_alloc, dest + idx,
                                                 std::move_if_noexcept(src[idx]));
            }
        } catch (...) {
            ++idx;
            for (; idx < count; ++idx) {
                allocator_traits_type::destroy(_alloc, dest + idx);
            }
            throw;
        }

        for (; idx < count; ++idx) {
            allocator_traits_type::destroy(_alloc, src + idx);
        }

        idx = 0;
        try {
            for (; idx < count; ++idx) {
                allocator_traits_type::construct(_alloc, src + idx, value);
            }
        } catch (...) {
            --idx;
            for (; idx >= 0; --idx) {
                allocator_traits_type::destroy(_alloc, src + idx);
            }
            for (; idx < count; ++idx) {
                allocator_traits_type::construct(_alloc, src + idx,
                                                 std::move_if_noexcept(dest[idx]));
            }
            for (; idx >= 0; --idx) {
                allocator_traits_type::destroy(_alloc, dest + idx);
            }

            throw;
        }

        return src;
    }

    const_pointer data() const noexcept {
        return _start;
    }

    size_type size() const noexcept {
        return _finish - _start;
    }

    size_type capacity() const noexcept {
        return _endofstorage - _start;
    }

    bool empty() const noexcept {
        return _start == _finish;
    }

    iterator begin() const noexcept {
        return _start;
    }

    iterator end() const noexcept {
        return _finish;
    }
};
}  // namespace wind
