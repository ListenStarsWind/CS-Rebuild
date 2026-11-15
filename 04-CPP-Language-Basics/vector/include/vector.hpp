#pragma once

#include <algorithm>
#include <list>
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
    using self_type = vector<value_type, allocator_type>;

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

   private:
    // 默认追加指定数目元素 _finish 会进行适应性调整
    void append_default(size_type count) {
        if (count == 0) return;

        const size_type maxSize = allocator_traits_type::max_size(_alloc);

        size_type oldSize = size();
        if (count > maxSize - oldSize) throw std::length_error("wind::vector::append_default");

        reserve(oldSize + count);

        size_type idx = 0;

        try {
            for (; idx < count; ++idx) {
                allocator_traits_type::construct(_alloc, _finish + idx);
            }
        } catch (...) {
            while (idx > 0) {
                --idx;
                allocator_traits_type::destroy(_alloc, _finish + idx);
            }
            throw;
        }

        _finish += count;
    }

    // 析构并溢出末尾指定元素
    void pop_back_n(size_type count) noexcept {
        if (count == 0) return;

        size_type oldSize = size();
        if (count > oldSize) count = oldSize;
        pointer curr = _finish - count;
        while (curr != _finish) {
            allocator_traits_type::destroy(_alloc, curr);
            ++curr;
        }
        _finish -= count;
    }

   public:
    vector()
        : _start(nullptr), _finish(nullptr), _endofstorage(nullptr), _alloc(allocator_type()) {};

    vector(const self_type& v)
        : _start(nullptr),
          _finish(nullptr),
          _endofstorage(nullptr),
          _alloc(allocator_traits_type::select_on_container_copy_construction(v._alloc)) {
        try {
            insert(end(), v.begin(), v.end());
        } catch (...) {
            clear();
            if (_start) {
                allocator_traits_type::deallocate(_alloc, _start, capacity());
            }
            throw;
        }
    }

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

    void resize(size_type n) {
        size_type oldSize = size();

        if (n < oldSize) {
            pop_back_n(oldSize - n);
        } else {
            const size_type maxSize = allocator_traits_type::max_size(_alloc);
            append_default(n - oldSize);
        }
    }

    void resize(size_type n, const_reference val) {
        const size_type oldSize = size();

        if (n < oldSize) {
            pop_back_n(oldSize - n);
            return;
        }

        if (n == oldSize) return;

        const size_type count = n - oldSize;

        // 提前保存副本，避免扩容导致 val 引用失效
        value_type fill_value = val;

        // 默认构造新增对象，并更新 _finish
        append_default(count);

        try {
            pointer curr = _finish - count;

            while (curr != _finish) {
                *curr = fill_value;
                ++curr;
            }
        } catch (...) {
            // 移除本次新增的所有对象
            pop_back_n(count);
            throw;
        }
    }

    void clear() {
        pop_back_n(size());
    }

    // 在指定位置填充若干对象
    iterator insert(const_iterator pos, size_type count, const_reference value) {
        static_assert(std::is_default_constructible_v<value_type> &&
                          std::is_copy_constructible_v<value_type> &&
                          std::is_copy_assignable_v<value_type>,
                      "wind::vector::insert: value_type must support default construction, "
                      "copy construction and copy assignment");

        // 检查位置是否处于合法区间
        size_type oldSize = size();
        difference_type insert_offset = pos - _start;
        if (insert_offset < 0 || insert_offset > oldSize)
            throw std::out_of_range("wind::vector::insert: invalid iterator position");

        // 如果数目为0, 直接退出
        // 另外为了和返回值契合, 再取消 const
        if (count == 0) return const_cast<iterator>(pos);

        // 取出允许的最大容量上限, 检查是否存在溢出的可能
        const size_type maxSize = allocator_traits_type::max_size(_alloc);
        if (count > maxSize - oldSize)
            throw std::length_error("wind::vector::insert(iterator, size_type, const_reference)");

        // 拷贝填充元素, 避免出现自依赖问题
        // 此处强制要求存在拷贝构造
        value_type fill_value = value;

        // 要求拥有足够的容量
        reserve(oldSize + count);

        // 理想填充之后的情况
        // [0,insert_offset) 不变
        // [insert_offset, insert_offset + count) 新填充区间
        // [insert_offset + count, oldSize + count ) 新移动出的区间

        pointer insert_pos = _start + insert_offset;  // 填充区间起点
        pointer move_src = insert_pos;                // 移动源区间起点
        pointer move_dst = insert_pos + count;        // 移动目标区间起点
        // 注意这里可以尾插, 存在 0 的可能 之后 -1 可为负数
        difference_type move_count = oldSize - insert_offset;  // 需要移动的元素个数
        const difference_type overlap_count =
            move_count > count ? move_count - count : 0;  // 重叠的个数

        // 使用有符号, 防止下溢出
        difference_type idx = 0;

        // 确保理想填充的合法区间中都是有效对象
        try {
            for (; idx < count; ++idx) {
                // 不输入第三参数, 完美转发后命中无参构造
                allocator_traits_type::construct(_alloc, _finish + idx);
            }
        } catch (...) {
            // 空构造失败进行状态回退, 对已经构造的进行析构
            for (--idx; idx >= 0; --idx) {
                allocator_traits_type::destroy(_alloc, _finish + idx);
            }
            throw;
        }

        idx = move_count - 1;
        // 抛出异常进行回退
        try {
            // 类型特征
            // std::is_nothrow_move_assignable_v<T> : 判断类型 T 的移动赋值操作是否保证不抛异常
            // std::is_copy_assignable_v<T> : 判断类型 T 是否支持拷贝赋值

            // 如果移动赋值保证不抛异常, 优先选择移动赋值
            // 如果没有拷贝赋值, 即使可能抛出异常, 也被迫使用移动赋值

            // 由于可能是重叠空间, 需要从后往前挪动
            // 进行数据载体的等效化处理
            for (; idx >= 0; --idx) {
                if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                    move_dst[idx] = std::move(move_src[idx]);
                } else {
                    move_dst[idx] = move_src[idx];
                }
            }
        } catch (...) {
            // 回退
            // 1) 源数据恢复

            // 如果抛出异常, 只有一种情况
            // 1. 拷贝赋值抛出异常, 原先的数据载体没有被移动
            // 2. 对方的保证有问题, 实际抛出异常, 这不是我们的问题, 不考虑
            // 3. 没有拷贝赋值, 使用移动赋值抛出异常, 但是不可能, 这样通不过编译器判断, 不考虑

            // 如果中途出现异常, 进行状态回退

            // 1. 如果是移动语义, 源后缀数据被移动, 需要移回(不考虑)
            // 2. 如果是拷贝赋值, 除非因为源后缀和目的前缀重叠, 否则源数据不会丧失

            if (idx < overlap_count) {
                for (difference_type i = idx + count; i < move_count; ++i) {
                    if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                        move_src[i] = std::move(move_dst[i]);
                    } else {
                        move_src[i] = move_dst[i];
                    }
                }
            }

            // 2) 将额外构造的, 原先准备被赋值的新元素析构
            for (idx = 0; idx < count; ++idx) {
                allocator_traits_type::destroy(_alloc, _finish + idx);
            }
            throw;
        }

        // 开始填充
        // 抛出异常同样回退
        try {
            for (idx = 0; idx < count; ++idx) {
                // 强制要求支持拷贝赋值
                insert_pos[idx] = fill_value;
            }
        } catch (...) {
            // 抛出异常开始回退

            // 1. 让原来的元素恢复原先是数据状态
            for (idx = 0; idx < move_count; ++idx) {
                if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                    move_src[idx] = std::move(move_dst[idx]);
                } else {
                    move_src[idx] = move_dst[idx];
                }
            }

            // 2.析构额外构造出的新元素
            for (idx = 0; idx < count; ++idx) {
                allocator_traits_type::destroy(_alloc, _finish + idx);
            }
            throw;
        }

        _finish += count;
        return insert_pos;
    }

    iterator erase(const_iterator first, const_iterator last) {
        if (first < _start || first > _finish)
            throw std::out_of_range("wind::vector::erase(range)");

        if (first > last || last > _finish) throw std::out_of_range("wind::vector::erase(range)");

        if (first == last) return const_cast<iterator>(first);

        pointer move_dst = const_cast<pointer>(first);
        pointer move_src = const_cast<pointer>(last);
        difference_type erase_count = last - first;
        difference_type move_count = _finish - last;
        pointer dest_pos = _finish - erase_count;

        difference_type idx = 0;
        try {
            for (; idx < move_count; ++idx) {
                if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                    move_dst[idx] = std::move(move_src[idx]);
                } else {
                    move_dst[idx] = move_src[idx];
                }
            }
        } catch (...) {
            for (difference_type i = idx - erase_count; i >= 0; --i) {
                if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                    move_src[i] = std::move(move_dst[i]);
                } else {
                    move_src[i] = move_dst[i];
                }
            }
            throw;
        }

        for (idx = 0; idx < erase_count; ++idx) {
            allocator_traits_type::destroy(_alloc, dest_pos + idx);
        }

        _finish = dest_pos;
        return move_dst;
    }

    // 插入初始化列表
    iterator insert(const_iterator pos, std::initializer_list<value_type> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }

    // 在特定位置拷贝式插入一个对象
    iterator insert(const_iterator pos, const_reference value) {
        return emplace(pos, value);
    }

    // 在特定位置移动式插入一个对象
    iterator insert(const_pointer pos, value_type&& value) {
        return emplace(pos, std::move(value));
    }

    // 在特定位置移动式插入一个对象
    template <class... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        static_assert(std::is_default_constructible_v<value_type> &&
                          std::is_copy_constructible_v<value_type> &&
                          std::is_copy_assignable_v<value_type>,
                      "wind::vector::emplace: value_type does not satisfy current implementation "
                      "requirements");

        size_type oldSize = size();
        difference_type insert_offset = pos - _start;
        if (insert_offset < 0 || insert_offset > oldSize)
            throw std::out_of_range(
                "wind::vector::emplace(const_iterator, ...): invalid iterator position");

        const size_type maxSize = allocator_traits_type::max_size(_alloc);
        if (1 > maxSize - oldSize)
            throw std::length_error("wind::vector::emplace(const_iterator, ...)");

        reserve(oldSize + 1);

        pointer insert_pos = _start + insert_offset;
        pointer move_src = insert_pos;
        pointer move_dst = insert_pos + 1;
        difference_type move_count = oldSize - insert_offset;
        const difference_type overlap_count = move_count > 1 ? move_count - 1 : 0;  // 重叠的个数

        difference_type idx = 0;

        allocator_traits_type::construct(_alloc, _finish);

        idx = move_count - 1;
        try {
            for (; idx >= 0; --idx) {
                if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                    move_dst[idx] = std::move(move_src[idx]);
                } else {
                    move_dst[idx] = move_src[idx];
                }
            }
        } catch (...) {
            if (idx < overlap_count) {
                for (difference_type i = idx + 1; i < move_count; ++i) {
                    if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                        move_src[i] = std::move(move_dst[i]);
                    } else {
                        move_src[i] = move_dst[i];
                    }
                }
            }

            allocator_traits_type::destroy(_alloc, _finish);

            throw;
        }

        try {
            *insert_pos = value_type(std::forward<Args>(args)...);
        } catch (...) {
            for (idx = 0; idx < move_count; ++idx) {
                if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                    move_src[idx] = std::move(move_dst[idx]);
                } else {
                    move_src[idx] = move_dst[idx];
                }
            }
            allocator_traits_type::destroy(_alloc, _finish);
            throw;
        }

        ++_finish;
        return insert_pos;
    }

    template <std::input_iterator InputIterator>
    iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
        static_assert(std::is_default_constructible_v<value_type> &&
                          std::is_copy_constructible_v<value_type> &&
                          std::is_copy_assignable_v<value_type>,
                      "wind::vector::insert(range): value_type must support default construction, "
                      "copy construction and copy assignment");

        size_type oldSize = size();
        difference_type insert_offset = pos - _start;
        if (insert_offset < 0 || insert_offset > oldSize)
            throw std::out_of_range("wind::vector::insert(range): invalid iterator position");

        std::list<value_type, allocator_type> tmp(first, last);
        size_type count = tmp.size();

        if (count == 0) return const_cast<iterator>(pos);

        const size_type maxSize = allocator_traits_type::max_size(_alloc);
        if (count > maxSize - oldSize)
            throw std::length_error("wind::vector::insert(range): requested size exceeds max_size");

        reserve(oldSize + count);

        pointer insert_pos = _start + insert_offset;
        pointer move_src = insert_pos;
        pointer move_dst = insert_pos + count;
        difference_type move_count = oldSize - insert_offset;
        const difference_type overlap_count = move_count > count ? move_count - count : 0;

        difference_type idx = 0;

        try {
            for (; idx < count; ++idx) {
                allocator_traits_type::construct(_alloc, _finish + idx);
            }
        } catch (...) {
            for (--idx; idx >= 0; --idx) {
                allocator_traits_type::destroy(_alloc, _finish + idx);
            }
            throw;
        }

        idx = move_count - 1;
        try {
            for (; idx >= 0; --idx) {
                if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                    move_dst[idx] = std::move(move_src[idx]);
                } else {
                    move_dst[idx] = move_src[idx];
                }
            }
        } catch (...) {
            if (idx < overlap_count) {
                for (difference_type i = idx + count; i < move_count; ++i) {
                    if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                        move_src[i] = std::move(move_dst[i]);
                    } else {
                        move_src[i] = move_dst[i];
                    }
                }
            }

            for (idx = 0; idx < count; ++idx) {
                allocator_traits_type::destroy(_alloc, _finish + idx);
            }
            throw;
        }

        try {
            auto it = tmp.begin();
            for (idx = 0; idx < count; ++idx) {
                if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                    insert_pos[idx] = std::move(*it);
                } else {
                    insert_pos[idx] = *it;
                }
                ++it;
            }
        } catch (...) {
            for (idx = 0; idx < move_count; ++idx) {
                if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                    move_src[idx] = std::move(move_dst[idx]);
                } else {
                    move_src[idx] = move_dst[idx];
                }
            }

            for (idx = 0; idx < count; ++idx) {
                allocator_traits_type::destroy(_alloc, _finish + idx);
            }
            throw;
        }

        _finish += count;
        return insert_pos;
    }

    iterator erase(const_iterator pos) {
        if (pos < _start || pos >= _finish)
            throw std::out_of_range("wind::vector::erase(pos): invalid iterator position");

        return erase(pos, pos + 1);
    }

    // 为了防止自引用的 emplace 命中上述函数模板
    // 专门特化两个重载函数, 复用以往的接口
    iterator emplace(const_iterator pos, reference value) {
        return insert(pos, 1, value);
    }

    iterator emplace(const_iterator pos, const_reference value) {
        return insert(pos, 1, value);
    }

    void push_back(const_reference val) {
        insert(end(), val);
    }

    void push_back(value_type&& val) {
        insert(end(), std::move(val));
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
