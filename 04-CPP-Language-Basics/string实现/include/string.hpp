#pragma once

#include <algorithm>
#include <boost/assert.hpp>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>

namespace wind {
class string {
   public:
    // 对自己进行重定义
    // 从而获得更明确的代称
    // 另一种传统写法是
    // typedef string self;
    using self = string;
    using iterator = char*;
    using const_iterator = const char*;

    friend std::istream& operator>>(std::istream& in, self& str);
    friend std::ostream& operator<<(std::ostream& out, const self& str);

    string() : _storage(storage()), _size(0), _capacity(string::SSO_CAPACITY) {}

    // 此处的构造函数使用了委托构造
    // 即允许一个构造函数通过初始化列表
    // 调用同一个类的另一个构造函数，
    // 复用其初始化逻辑
    string(const char* str) : string() {
        append(str);
    }

    // 迭代器区间构造
    template <class InputIterator>
        requires std::input_iterator<InputIterator>
    string(InputIterator first, InputIterator last) : string() {
        for (; first != last; ++first) append(1, *first);
    }

    string(const self& obj) : string() {
        reserve(obj._size);
        _size = obj._size;
        std::memcpy(data(), obj.data(), obj._size);
        data()[_size] = '\0';
    }

    self& operator=(const self& obj) {
        // 避免自赋值
        if (this == &obj) return *this;

        clear_content();

        //  确保容量充足，避免扩容时复制即将被覆盖的旧内容
        reserve(obj._size);

        // 复制有效字符
        std::memcpy(data(), obj.data(), obj._size);

        // 更新长度，统一补充终止符
        _size = obj._size;
        data()[_size] = '\0';

        return *this;
    }

    ~string() {
        release_storage();
    }

   public:
    static constexpr size_t npos = -1;

   private:
    static constexpr std::size_t SSO_CAPACITY = 15;
    static constexpr size_t max_capacity = std::numeric_limits<size_t>::max() - 1;
    union storage {
        // 注意末尾还有一个 '\0' , 所以加1
        char _buffer[SSO_CAPACITY + 1];
        char* _ptr;
    };

    // 对于成员函数, 我们先设置缺省值
    // 如果确实有必要, 再用初始化列表进行修改
    storage _storage{};  // 这是空列表初始化, 明确要求编译器对联合体第一个成员进行清零,
                         // 由于其是最大成员, 顺带也给可能的 _ptr 一并初始化为0了
    size_t _size = 0;    // 最开始没有字符串, 长度为0
    size_t _capacity = string::SSO_CAPACITY;  // 最开始使用 SSO , 缓冲区大小固定

    // 内部方法 通过容量的大小判断使用哪种模式
    // const 表示不对对象进行写操作
    // const 限制成员函数对对象状态的修改
    // noexcept 表示函数承诺不让异常向外传播
    // 如果异常仍然逃出该函数，程序将调用 std::terminate()
    bool is_sso() const noexcept {
        return _capacity == string::SSO_CAPACITY;
    }

    // 申请指定容量的堆空间
    static char* allocate(size_t capacity) {
        return new char[capacity + 1];
    }

    // 释放堆空间
    static void deallocate(char* ptr) noexcept {
        delete[] ptr;
    }

    // 依据上层指示进行严格扩容
    void grow_capacity(size_t new_capacity) {
        BOOST_ASSERT_MSG(
            new_capacity > _capacity,
            "This function requires a capacity greater than the current capacity.");  // 此函数在设计上不负责缩小容量
        BOOST_ASSERT_MSG(
            new_capacity <= max_capacity,
            "This function does not perform capacity overflow checks.");  // 此函数在设计上不负责容量溢出检查

        char* new_ptr = allocate(new_capacity);

        std::memcpy(new_ptr, data(), _size + 1);

        if (!is_sso()) deallocate(_storage._ptr);

        _storage._ptr = new_ptr;
        _capacity = new_capacity;
    }

    // 依据上层指示进行严格缩容
    void shrink_capacity(size_t target_capacity) {
        BOOST_ASSERT_MSG(!is_sso(), "This function requires heap storage.");

        BOOST_ASSERT_MSG(target_capacity < _capacity,
                         "This function requires a capacity smaller than the current capacity.");

        BOOST_ASSERT_MSG(target_capacity >= _size,
                         "This function does not support shrinking that causes data loss.");

        if (target_capacity <= SSO_CAPACITY) {
            char* old_ptr = _storage._ptr;

            std::memcpy(_storage._buffer, old_ptr, _size + 1);

            deallocate(old_ptr);
            _capacity = SSO_CAPACITY;
            return;
        }

        char* new_ptr = allocate(target_capacity);

        std::memcpy(new_ptr, _storage._ptr, _size + 1);

        deallocate(_storage._ptr);

        _storage._ptr = new_ptr;
        _capacity = target_capacity;
    }

    // 清空字符串内容，保留现有存储空间
    void clear_content() noexcept {
        _size = 0;
        data()[0] = '\0';
    }

    // 释放额外存储空间，并恢复对象的初始状态
    // 一个主要用途是给析构做清理调用
    // 我们称之为"怎么来的, 就怎么走"
    void release_storage() noexcept {
        if (!is_sso()) deallocate(_storage._ptr);

        _storage._buffer[0] = '\0';
        _size = 0;
        _capacity = SSO_CAPACITY;
    }

    // 朴素匹配：逐一尝试可能的匹配起点
    size_t naive_find(const char* chars, size_t pos, size_t n) const noexcept {
        // 检查起始位置
        if (pos > _size) return npos;

        // 空模式串在合法起点直接匹配成功
        if (n == 0) return pos;

        // 剩余字符不足以容纳模式串
        if (n > _size - pos) return npos;

        // 遍历所有可能的匹配起点
        for (size_t i = pos; i <= _size - n; ++i) {
            if (std::memcmp(data() + i, chars, n) == 0) return i;
        }

        return npos;
    }

    // KMP 匹配：利用 LPS 表减少重复比较
    size_t kmp_find(const char* chars, size_t pos, size_t n) const {
        // 检查搜索范围
        if (pos > _size) return npos;
        if (n == 0) return pos;
        if (n > _size - pos) return npos;

        // 构造 LPS 表，记录各前缀的最长相等真前后缀长度
        std::vector<size_t> lps(n, 0);

        for (size_t i = 1, matched = 0; i < n;) {
            if (chars[i] == chars[matched]) {
                lps[i++] = ++matched;
            } else if (matched != 0) {
                // 根据已经匹配的信息回退
                matched = lps[matched - 1];
            } else {
                lps[i++] = 0;
            }
        }

        // 正式进行 KMP 匹配
        size_t i = pos;
        size_t j = 0;

        while (i < _size) {
            if (data()[i] == chars[j]) {
                ++i;
                ++j;

                // 完整匹配，返回相对于整个字符串的起始下标
                if (j == n) return i - n;
            } else if (j != 0) {
                // 主串位置保持不变，只调整模式串位置
                j = lps[j - 1];
            } else {
                ++i;
            }
        }

        return npos;
    }

    // 按字典序比较两个字符串
    int compare(const self& obj) const noexcept {
        // 先比较双方共同具有的字符
        const size_t len = std::min(_size, obj._size);

        int result = std::memcmp(data(), obj.data(), len);

        if (result != 0) return result;

        // 公共部分相同，长度较短的字符串更小
        if (_size < obj._size) return -1;
        if (_size > obj._size) return 1;

        return 0;
    }

   public:
    size_t size() const noexcept {
        return _size;
    }

    size_t capacity() const noexcept {
        return _capacity;
    }

    void clear() {
        clear_content();
    }

    void reset_storage() {
        release_storage();
    }

    // 下标访问，返回可修改的字符引用
    char& operator[](size_t pos) noexcept {
        BOOST_ASSERT(pos < _size);
        return data()[pos];
    }

    // const 对象只能通过常引用访问字符
    const char& operator[](size_t pos) const noexcept {
        BOOST_ASSERT(pos < _size);
        return data()[pos];
    }

    char* data() noexcept {
        return is_sso() ? _storage._buffer : _storage._ptr;
    }

    const char* data() const noexcept {
        return is_sso() ? _storage._buffer : _storage._ptr;
    }

    // 带边界检查的下标访问
    char& at(size_t pos) {
        if (pos >= _size) throw std::out_of_range("string::at: index out of range");

        return (*this)[pos];
    }

    // const 重载，返回常引用
    const char& at(size_t pos) const {
        if (pos >= _size) throw std::out_of_range("string::at: index out of range");

        return (*this)[pos];
    }

    // 获取首元素迭代器
    iterator begin() noexcept {
        return data();
    }

    // 获取尾后迭代器
    iterator end() noexcept {
        return data() + _size;
    }

    // const 对象的迭代器接口
    const_iterator begin() const noexcept {
        return data();
    }

    const_iterator end() const noexcept {
        return data() + _size;
    }

    // 显式获取只读迭代器
    const_iterator cbegin() const noexcept {
        return data();
    }

    const_iterator cend() const noexcept {
        return data() + _size;
    }

    void reserve(size_t min_capacity) {
        if (min_capacity <= _capacity) return;

        if (min_capacity > max_capacity) throw std::length_error("string capacity overflow");

        // 按 1.5 倍计算，避免整数溢出
        size_t increment = _capacity / 2;

        size_t new_capacity =
            _capacity > max_capacity - increment ? max_capacity : _capacity + increment;

        new_capacity = std::max(new_capacity, min_capacity);

        grow_capacity(new_capacity);
    }

    void shrink_to_fit() {
        if (is_sso()) return;

        if (_capacity == _size) return;

        shrink_capacity(_size);
    }

    // 从字符数组中追加 n 个字符，允许包含 '\0'
    self& append(const char* chars, size_t n) {
        if (n == 0) return *this;

        // 检查追加后的容量是否溢出
        if (n > max_capacity - _size) throw std::length_error("string capacity overflow");

        // 判断源地址是否位于当前字符串的有效存储范围内
        const auto begin_addr = reinterpret_cast<std::uintptr_t>(data());
        const auto end_addr = reinterpret_cast<std::uintptr_t>(data() + _size);
        const auto source_addr = reinterpret_cast<std::uintptr_t>(chars);

        const bool self_reference = source_addr >= begin_addr && source_addr <= end_addr;

        // 如果存在自引用，记录源地址相对于当前字符串的偏移量
        const size_t offset = self_reference ? static_cast<size_t>(source_addr - begin_addr) : 0;

        // 根据追加后的字符数量决定是否扩容
        reserve(_size + n);

        // 扩容可能导致原地址失效，因此需要重新定位自引用源地址
        const char* source = self_reference ? data() + offset : chars;

        // 复制指定数量的字符，允许源区域与目标区域重叠
        std::memmove(data() + _size, source, n);

        // 更新长度，统一补充字符串结束符
        _size += n;
        data()[_size] = '\0';

        return *this;
    }

    self& append(const char* str) {
        return append(str, std::strlen(str));
    }

    // 从另一个 string 的指定位置开始，追加最多 sublen 个字符
    self& append(const self& str, size_t subpos, size_t sublen = npos) {
        // 起始位置不能超过源字符串长度
        if (subpos > str._size) throw std::out_of_range("string::append: subpos out of range");

        // 根据剩余字符数量确定实际追加长度
        const size_t len = std::min(sublen, str._size - subpos);

        // 复用字符数组追加接口
        return append(str.data() + subpos, len);
    }

    // 追加另一个 string 的全部内容
    self& append(const self& str) {
        return append(str, 0);
    }

    // 在字符串末尾追加 n 个字符 c
    self& append(size_t n, char c) {
        if (n == 0) return *this;

        // 检查追加后的容量是否溢出
        if (n > max_capacity - _size) throw std::length_error("string capacity overflow");

        // 确保具有足够的存储空间
        reserve(_size + n);

        // 将 n 个字符 c 填充到字符串末尾
        std::fill_n(data() + _size, n, c);

        // 更新长度，并补充字符串结束符
        _size += n;
        data()[_size] = '\0';

        return *this;
    }

    // 追加迭代器区间中的字符
    template <std::input_iterator InputIterator>
    self& append(InputIterator first, InputIterator last) {
        // 先构造独立的临时字符串，避免自引用时迭代器失效
        self temp(first, last);

        // 复用已有的字符串追加接口
        return append(temp);
    }

    // 追加初始化列表中的所有字符
    self& append(std::initializer_list<char> il) {
        return append(il.begin(), il.size());
    }

    // 追加能够转换为字符串视图的字符序列
    template <class StringViewLike>
        requires(std::convertible_to<const StringViewLike&, std::string_view> &&
                 !std::convertible_to<const StringViewLike&, const char*>)
    self& append(const StringViewLike& t) {
        std::string_view view = t;
        return append(view.data(), view.size());
    }

    // 追加字符串视图中指定位置开始的部分字符
    template <class StringViewLike>
        requires(std::convertible_to<const StringViewLike&, std::string_view> &&
                 !std::convertible_to<const StringViewLike&, const char*>)
    self& append(const StringViewLike& t, size_t pos, size_t count = npos) {
        std::string_view view = t;
        std::string_view subview = view.substr(pos, count);
        return append(subview.data(), subview.size());
    }

    void push_back(char c) {
        append(1, c);
    }

    // 追加另一个 string 对象
    self& operator+=(const self& str) {
        return append(str);
    }

    // 追加 C 风格字符串
    self& operator+=(const char* str) {
        return append(str);
    }

    // 追加单个字符
    self& operator+=(char c) {
        return append(1, c);
    }

    // 追加初始化列表中的字符
    self& operator+=(std::initializer_list<char> il) {
        return append(il);
    }

    void swap(self& obj) noexcept {
        std::swap(_storage, obj._storage);
        std::swap(_size, obj._size);
        std::swap(_capacity, obj._capacity);
    }

    self& insert(size_t pos, const char* chars, size_t n) {
        // 检查插入位置
        if (pos > _size) throw std::out_of_range("string::insert: pos out of range");

        if (n == 0) return *this;

        // 检查插入后的容量是否溢出
        if (n > max_capacity - _size) throw std::length_error("string capacity overflow");

        // 通过地址区间判断是否存在自引用
        const auto begin_addr = reinterpret_cast<std::uintptr_t>(data());
        const auto source_addr = reinterpret_cast<std::uintptr_t>(chars);

        const bool self_reference = source_addr >= begin_addr && source_addr - begin_addr <= _size;

        // 如果存在自引用，记录源地址相对于当前字符串的偏移量
        const size_t offset = self_reference ? static_cast<size_t>(source_addr - begin_addr) : 0;

        // 确保具有足够的容量
        reserve(_size + n);

        // 将插入位置之后的有效字符整体右移
        std::memmove(data() + pos + n, data() + pos, _size - pos);

        // 利用两个视图描述需要复制的字符区域
        std::string_view a{};
        std::string_view b{};

        if (!self_reference) {
            // 外部字符数组，直接使用原始数据
            a = std::string_view(chars, n);
        } else if (offset + n <= pos) {
            // 源区域完全位于插入位置之前，不受尾部移动影响
            a = std::string_view(data() + offset, n);
        } else if (offset >= pos) {
            // 源区域整体向右移动了 n 个位置
            a = std::string_view(data() + offset + n, n);
        } else {
            // 插入位置将原来的源区域分成两段
            const size_t first_len = pos - offset;

            a = std::string_view(data() + offset, first_len);
            b = std::string_view(data() + pos + n, n - first_len);
        }

        // 将源字符复制到插入位置
        std::memcpy(data() + pos, a.data(), a.size());

        if (!b.empty()) std::memcpy(data() + pos + a.size(), b.data(), b.size());

        // 更新字符串长度，并统一添加终止符
        _size += n;
        data()[_size] = '\0';

        return *this;
    }

    self& insert(size_t pos, const self& str) {
        return insert(pos, str.data(), str.size());
    }

    self& insert(size_t pos, const self& str, size_t subpos, size_t sublen = npos) {
        // 检查子串开始位置
        if (subpos > str._size) throw std::out_of_range("string::insert: subpos out of range");

        // 根据剩余字符数量确定实际插入长度
        const size_t len = std::min(sublen, str._size - subpos);

        return insert(pos, str.data() + subpos, len);
    }

    self& insert(size_t pos, const char* str) {
        return insert(pos, str, std::strlen(str));
    }

    self& insert(size_t pos, size_t n, char c) {
        // 检查插入位置
        if (pos > _size) throw std::out_of_range("string::insert: pos out of range");

        if (n == 0) return *this;

        // 检查插入后的容量是否溢出
        if (n > max_capacity - _size) throw std::length_error("string capacity overflow");

        reserve(_size + n);

        // 将插入位置之后的有效字符整体右移
        std::memmove(data() + pos + n, data() + pos, _size - pos);

        // 将 n 个字符 c 填充到插入起始位置
        std::fill_n(data() + pos, n, c);

        _size += n;
        data()[_size] = '\0';

        return *this;
    }

    iterator insert(const_iterator p, size_t n, char c) {
        size_t pos = static_cast<size_t>(p - cbegin());

        insert(pos, n, c);

        return data() + pos;
    }

    iterator insert(const_iterator p, char c) {
        return insert(p, 1, c);
    }

    template <std::input_iterator InputIterator>
    iterator insert(const_iterator p, InputIterator first, InputIterator last) {
        size_t pos = static_cast<size_t>(p - cbegin());
        string tmp(first, last);
        insert(pos, tmp);
        return data() + pos;
    }

    iterator insert(const_iterator p, std::initializer_list<char> il) {
        size_t pos = static_cast<size_t>(p - cbegin());
        insert(pos, il.begin(), il.size());
        return data() + pos;
    }

    // 插入整个字符串视图
    template <class StringViewLike>
        requires(std::convertible_to<const StringViewLike&, std::string_view> &&
                 !std::convertible_to<const StringViewLike&, const char*>)
    self& insert(size_t pos, const StringViewLike& t) {
        std::string_view view = t;
        return insert(pos, view.data(), view.size());
    }

    // 插入字符串视图中的指定部分
    template <class StringViewLike>
        requires(std::convertible_to<const StringViewLike&, std::string_view> &&
                 !std::convertible_to<const StringViewLike&, const char*>)
    self& insert(size_t pos, const StringViewLike& t, size_t subpos, size_t sublen = npos) {
        std::string_view view = t;
        std::string_view subview = view.substr(subpos, sublen);

        return insert(pos, subview.data(), subview.size());
    }

    // 从指定位置开始，删除最多 count 个字符
    self& erase(size_t pos = 0, size_t count = npos) {
        // 检查删除位置
        if (pos > _size) throw std::out_of_range("string::erase: pos out of range");

        // 确定实际删除的字符数量
        size_t len = std::min(count, _size - pos);

        // 将删除区间之后的有效字符向前移动
        std::memmove(data() + pos, data() + pos + len, _size - pos - len);

        // 更新长度，统一补充终止符
        _size -= len;
        data()[_size] = '\0';

        return *this;
    }

    // 删除迭代器指向的单个字符
    iterator erase(const_iterator p) {
        size_t pos = static_cast<size_t>(p - cbegin());

        erase(pos, 1);

        return data() + pos;
    }

    // 删除指定迭代器区间内的字符
    iterator erase(const_iterator first, const_iterator last) {
        size_t pos = static_cast<size_t>(first - cbegin());
        size_t len = static_cast<size_t>(last - first);

        erase(pos, len);

        return data() + pos;
    }

    void resize(size_t n, char c = '\0') {
        reserve(n);
        for (size_t i = _size; i < n; i++) {
            data()[i] = c;
        }
        data()[n] = '\0';
        _size = n;
    }

    // 查找另一个 string 对象
    size_t find(const self& str, size_t pos = 0) const noexcept {
        return naive_find(str.data(), pos, str.size());
    }

    // 查找字符数组中的指定数量的字符
    size_t find(const char* chars, size_t pos, size_t n) const {
        return naive_find(chars, pos, n);
    }

    // 查找 C 风格字符串
    size_t find(const char* str, size_t pos = 0) const {
        return find(str, pos, std::strlen(str));
    }

    // 查找单个字符
    size_t find(char c, size_t pos = 0) const noexcept {
        return naive_find(&c, pos, 1);
    }

    // 查找能够转换为字符串视图的字符序列
    // 这里用到了条件 noexcept, 满足括号中的条件才真正生效
    // is_nothrow_convertible_v 是C++20中一个新的特性,用来判断
    // 一个类型隐式转为另一个类型是否可能会抛出异常
    template <class StringViewLike>
        requires(std::convertible_to<const StringViewLike&, std::string_view> &&
                 !std::convertible_to<const StringViewLike&, const char*>)
    size_t find(const StringViewLike& t, size_t pos = 0) const
        noexcept(std::is_nothrow_convertible_v<const StringViewLike&, std::string_view>) {
        std::string_view view = t;
        return naive_find(view.data(), pos, view.size());
    }

    // 返回从 pos 开始、最多包含 count 个字符的新字符串
    self substr(size_t pos = 0, size_t count = npos) const {
        // 检查子串起始位置
        if (pos > _size) throw std::out_of_range("string::substr: pos out of range");

        // 确定实际截取长度
        const size_t len = std::min(count, _size - pos);

        // 利用迭代器区间构造新字符串
        return self(cbegin() + pos, cbegin() + pos + len);
    }

    // 判断两个字符串是否相等
    bool operator==(const self& obj) const noexcept {
        return _size == obj._size && std::memcmp(data(), obj.data(), _size) == 0;
    }

    // 三路比较
    std::strong_ordering operator<=>(const self& obj) const noexcept {
        return compare(obj) <=> 0;
    }
};

inline std::istream& operator>>(std::istream& in, string::self& str) {
    constexpr size_t BUFFER_SIZE = 16;

    char buff[BUFFER_SIZE];
    size_t curr = 0;
    size_t extracted = 0;

    // 获取本次输入的最大字符数量
    const auto width = in.width();
    const size_t limit = width > 0 ? static_cast<size_t>(width) : string::npos - 1;

    // 准备格式化输入，并根据 skipws 设置处理前导空白
    std::istream::sentry sentry(in);

    if (!sentry) {
        in.width(0);
        return in;
    }

    // 开始读取前清空原有字符串，保留存储容量
    str.clear();

    while (extracted < limit) {
        // 查看下一个字符，但不从输入流中提取
        int ch = in.peek();

        // 输入结束或流读取失败
        if (ch == std::char_traits<char>::eof()) break;

        // 遇到空白字符，停止读取并将其保留在输入流中
        if (std::isspace(static_cast<char>(ch), in.getloc())) break;

        // 正式提取字符
        ch = in.get();

        if (ch == std::char_traits<char>::eof()) break;

        buff[curr++] = static_cast<char>(ch);
        ++extracted;

        // 缓冲区已满，将数据提交给字符串
        if (curr == BUFFER_SIZE) {
            str.append(buff, curr);
            curr = 0;
        }
    }

    // 提交缓冲区中剩余的字符
    if (curr != 0) str.append(buff, curr);

    // 恢复流宽度设置
    in.width(0);

    // 本次没有提取任何字符，标记输入失败
    if (extracted == 0) in.setstate(std::ios::failbit);

    return in;
}

inline std::ostream& operator<<(std::ostream& out, const string::self& str) {
    // 获取当前字段宽度和字符串长度
    const auto width = out.width();
    const auto length = str.size();

    // 计算填充字符数量
    const size_t padding =
        width > 0 && static_cast<size_t>(width) > length ? static_cast<size_t>(width) - length : 0;

    // 检查是否为左对齐
    const bool left = (out.flags() & std::ios_base::adjustfield) == std::ios_base::left;

    // 获取填充字符
    const char fill = out.fill();

    // 清除本次格式化输出的字段宽度
    out.width(0);

    // 填充指定数量的字符
    auto write_padding = [&]() {
        for (size_t i = 0; i < padding && out; ++i) out.put(fill);
    };

    // 默认右对齐：先填充
    if (!left) write_padding();

    // 输出字符串的完整字符序列
    if (out && length != 0) out.write(str.data(), static_cast<std::streamsize>(length));

    // 左对齐：后填充
    if (left && out) write_padding();

    return out;
}

inline std::istream& getline(std::istream& in, string& str, char delim = '\n') {
    constexpr size_t BUFFER_SIZE = 16;

    // 不跳过前导空白
    std::istream::sentry sentry(in, true);
    if (!sentry) return in;

    char buff[BUFFER_SIZE];
    size_t curr = 0;
    bool extracted = false;

    str.clear();

    while (true) {
        // 观察下一个字符
        int ch = in.peek();
        if (ch == std::char_traits<char>::eof()) break;

        // 提取字符
        ch = in.get();
        if (ch == std::char_traits<char>::eof()) break;

        extracted = true;

        // 分隔符被提取，但不保存
        if (static_cast<char>(ch) == delim) break;

        buff[curr++] = static_cast<char>(ch);

        if (curr == BUFFER_SIZE) {
            str.append(buff, curr);
            curr = 0;
        }
    }

    // 提交剩余字符
    if (curr != 0) str.append(buff, curr);

    // 没有成功提取任何字符
    if (!extracted) in.setstate(std::ios::failbit);

    return in;
}
}  // namespace wind
