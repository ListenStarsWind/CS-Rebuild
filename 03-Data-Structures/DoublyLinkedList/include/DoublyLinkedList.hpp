#pragma once

#include <cstddef>

/*
 * 循环双向链表（C++17）
 *
 * 对外使用面向过程的自由函数；内部节点采用非多态继承：
 *   NodeBase（prev/next）
 *     ├── SentinelNode（size）
 *     └── DLLNode<T>（data）
 *
 * 链表对象自身就是哨兵节点；空链表时哨兵的前后指针均指向自身。
 * 实现中仅在已经确定节点身份后，才允许从 NodeBase* 静态向下转型。
 *
 * 契约：
 * - DLL_Destroy(nullptr) 合法；其他链表指针必须有效。
 * - 传入的节点指针必须指向相应链表中存活的数据节点，不能是哨兵。
 * - 违反前置条件属于编程错误，由实现中的 Boost.Assert 尽力诊断。
 * - 删除节点立即销毁对象并释放内存，已有节点指针随之失效。
 * - 非空指针不代表有效指针；不会承诺检测悬空指针。
 *
 * 本文件声明模板接口。模板定义需放在本头文件中，或放在本头文件
 * 末尾包含的 .tpp 中，以便其他翻译单元实例化。
 */

namespace dll_detail
{
    // 仅负责双向链接；没有虚函数或虚析构函数。
    struct NodeBase
    {
        NodeBase* _prev;
        NodeBase* _next;
    };

    // 哨兵额外保存有效数据节点数量。
    struct SentinelNode : NodeBase
    {
        std::size_t _size;
    };
}

// 数据节点和哨兵共享 NodeBase 链接布局，数据类型由模板参数决定。
template<typename T>
struct DLLNode : dll_detail::NodeBase
{
    T data;
};

// 链表对象本身即哨兵节点，而不是另一个持有哨兵的管理对象。
template<typename T>
struct DoublyLinkedList : dll_detail::SentinelNode
{
};

/* ==================== 创建与销毁 ==================== */

/**
 * @brief 创建一个空的循环双向链表。
 * @return 新链表的哨兵节点指针。
 * @throws std::bad_alloc 当分配链表对象的存储空间失败时。
 * @post 成功时 size 为 0，哨兵的 prev/next 均指向自身。
 */
template<typename T>
DoublyLinkedList<T>* DLL_Create();

/**
 * @brief 销毁全部数据节点及哨兵节点。
 * @param list 链表指针；允许为 nullptr，此时不执行操作。
 * @note 销毁后，链表与节点的所有既有指针均失效。
 */
template<typename T>
void DLL_Destroy(DoublyLinkedList<T>* list);

/* ==================== 状态查询 ==================== */

/**
 * @brief 获取实际数据节点数量，不计入哨兵节点。
 * @pre list 必须有效。
 * @return 节点数量，时间复杂度 O(1)。
 */
template<typename T>
std::size_t DLL_Size(const DoublyLinkedList<T>* list);

/**
 * @brief 判断链表是否为空。
 * @pre list 必须有效。
 * @return 空链表返回 true，否则返回 false；时间复杂度 O(1)。
 */
template<typename T>
bool DLL_IsEmpty(const DoublyLinkedList<T>* list);

/* ==================== 定位与遍历 ==================== */

/**
 * @brief 获取第一个数据节点。
 * @pre list 必须有效。
 * @return 空链表返回 nullptr，否则返回首节点；O(1)。
 */
template<typename T>
DLLNode<T>* DLL_Front(DoublyLinkedList<T>* list);

/**
 * @brief 获取最后一个数据节点。
 * @pre list 必须有效。
 * @return 空链表返回 nullptr，否则返回尾节点；O(1)。
 */
template<typename T>
DLLNode<T>* DLL_Back(DoublyLinkedList<T>* list);

/**
 * @brief 获取指定数据节点的后继节点。
 * @pre list 有效；node 是 list 中存活的数据节点。
 * @return node 为尾节点时返回 nullptr，否则返回其后继；O(1)。
 * @note 对外不暴露哨兵节点。
 */
template<typename T>
DLLNode<T>* DLL_Next(DoublyLinkedList<T>* list, DLLNode<T>* node);

/**
 * @brief 获取指定数据节点的前驱节点。
 * @pre list 有效；node 是 list 中存活的数据节点。
 * @return node 为首节点时返回 nullptr，否则返回其前驱；O(1)。
 */
template<typename T>
DLLNode<T>* DLL_Prev(DoublyLinkedList<T>* list, DLLNode<T>* node);

/* ==================== 插入 ==================== */

/**
 * @brief 头插一个数据节点。
 * @pre list 必须有效。
 * @param value 用于构造新节点数据的值。
 * @return 成功时返回新节点；O(1)。
 * @throws std::bad_alloc 分配失败时；T 的复制构造也可能抛异常。
 * @note 发生异常时链表保持原状。
 */
template<typename T>
DLLNode<T>* DLL_PushFront(DoublyLinkedList<T>* list, const T& value);

/**
 * @brief 尾插一个数据节点。
 * @pre list 必须有效。
 * @return 成功时返回新节点；O(1)。
 * @throws std::bad_alloc 分配失败时；T 的复制构造也可能抛异常。
 * @note 发生异常时链表保持原状。
 */
template<typename T>
DLLNode<T>* DLL_PushBack(DoublyLinkedList<T>* list, const T& value);

/**
 * @brief 在 position 节点之前插入数据节点。
 * @pre list 有效；position 是 list 中存活的数据节点。
 * @return 成功时返回新节点；O(1)。
 * @throws std::bad_alloc 分配失败时；T 的复制构造也可能抛异常。
 * @note 发生异常时链表保持原状。
 */
template<typename T>
DLLNode<T>* DLL_InsertBefore(DoublyLinkedList<T>* list,
                             DLLNode<T>* position, const T& value);

/**
 * @brief 在 position 节点之后插入数据节点。
 * @pre list 有效；position 是 list 中存活的数据节点。
 * @return 成功时返回新节点；O(1)。
 * @throws std::bad_alloc 分配失败时；T 的复制构造也可能抛异常。
 * @note 发生异常时链表保持原状。
 */
template<typename T>
DLLNode<T>* DLL_InsertAfter(DoublyLinkedList<T>* list,
                            DLLNode<T>* position, const T& value);

/* ==================== 删除 ==================== */

/**
 * @brief 删除并销毁首个数据节点。
 * @pre list 有效且非空。
 * @note O(1)；原首节点的全部外部引用失效。
 */
template<typename T>
void DLL_PopFront(DoublyLinkedList<T>* list);

/**
 * @brief 删除并销毁最后一个数据节点。
 * @pre list 有效且非空。
 * @note O(1)；原尾节点的全部外部引用失效。
 */
template<typename T>
void DLL_PopBack(DoublyLinkedList<T>* list);

/**
 * @brief 删除并销毁指定数据节点。
 * @pre list 有效；node 是 list 中存活的数据节点。
 * @note O(1)，前提是调用者已保证节点归属及有效性。
 * @note 删除后 node 及其全部别名失效。
 */
template<typename T>
void DLL_Erase(DoublyLinkedList<T>* list, DLLNode<T>* node);

/**
 * @brief 删除所有数据节点，保留哨兵节点。
 * @pre list 必须有效。
 * @post 链表长度变为 0，哨兵恢复自环。
 * @note 空链表也允许调用；O(n)。
 */
template<typename T>
void DLL_Clear(DoublyLinkedList<T>* list);

/* ==================== 查找与数据访问 ==================== */

/**
 * @brief 查找第一个与 value 相等的数据节点。
 * @pre list 有效；T 支持相等比较（operator==）。
 * @return 找到时返回节点指针，否则返回 nullptr；O(n)。
 * @note 元素比较操作本身可能抛出异常。
 */
template<typename T>
DLLNode<T>* DLL_Find(DoublyLinkedList<T>* list, const T& value);

/**
 * @brief 获取指定节点的数据，只读引用，不发生复制。
 * @pre list 有效；node 是 list 中存活的数据节点。
 * @return 节点数据的常量引用；O(1)。
 * @note 节点删除后，返回的引用也立即失效。
 */
template<typename T>
const T& DLL_Get(const DoublyLinkedList<T>* list, const DLLNode<T>* node);

/**
 * @brief 修改指定节点的数据。
 * @pre list 有效；node 是 list 中存活的数据节点。
 * @pre T 支持从 const T& 进行赋值。
 * @note O(1) 次节点定位；数据赋值成本由 T 决定。
 * @note 赋值异常时的数据状态遵循 T 自身的异常保证。
 */
template<typename T>
void DLL_Set(DoublyLinkedList<T>* list, DLLNode<T>* node, const T& value);

// 待实现时启用：
// #include "DoublyLinkedList.tpp"
