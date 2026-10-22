#pragma once

#include <stddef.h>

typedef int SLDataType;

typedef struct SeqList SeqList;

typedef SeqList* pSeqList;
typedef const SeqList* const_pSeqList;

// ==================== 创建与销毁 ====================

// 创建顺序表对象并初始化内部成员
// 成功返回对象指针，失败返回 NULL
pSeqList SL_Create(void);

// 销毁顺序表，释放内部动态数组及对象本身
// 所有方法对象句柄obj都假设不为空
void SL_Destroy(pSeqList obj);

// ==================== 容量管理 ====================

// 确保顺序表容量至少为 capacity
// 若当前容量已满足要求，则不进行扩容
// 扩容或者不必调整, 被视为成功
// 成功返回0, 否则返回非0
int SL_Reserve(pSeqList obj, size_t capacity);

// 清空顺序表中的所有元素，将有效元素个数置为 0
// 保留已经分配的存储空间，不释放底层数组
void SL_Clear(pSeqList obj);

// 获取顺序表当前有效元素的个数
size_t SL_Size(const_pSeqList obj);

// 获取顺序表当前可容纳的最大元素个数
// 返回的是容量，而非已占用的元素个数
size_t SL_Capacity(const_pSeqList obj);

// 判断顺序表是否为空
// 返回 1 表示为空，返回 0 表示非空
int SL_IsEmpty(const_pSeqList obj);

// ==================== 插入 ====================

// 在顺序表尾部插入元素 x
// 容量不足时自动扩容
// 成功返回 0，失败返回非 0
int SL_PushBack(pSeqList obj, SLDataType x);

// 在顺序表头部插入元素 x
// 原有元素整体向后移动一个位置
// 容量不足时自动扩容
// 成功返回 0，失败返回非 0
int SL_PushFront(pSeqList obj, SLDataType x);

// 在指定下标 pos 处插入元素 x
// 原 pos 及其后面的元素整体向后移动
// pos 的有效范围为 [0, size]，允许在尾部插入
// 成功返回 0，失败返回非 0
// 容量不足时, 也会自动扩容
int SL_Insert(pSeqList obj, size_t pos, SLDataType x);

// ==================== 删除 ====================

// 删除顺序表尾部的一个元素
// 顺序表为空时删除失败
// 成功返回 0，失败返回非 0
int SL_PopBack(pSeqList obj);

// 删除顺序表头部的一个元素
// 后续元素整体向前移动一个位置
// 顺序表为空时删除失败
// 成功返回 0，失败返回非 0
int SL_PopFront(pSeqList obj);

// 删除指定下标 pos 处的元素
// 后续元素整体向前移动，填补删除位置
// pos 的有效范围为 [0, size)
// 成功返回 0，失败返回非 0
int SL_Erase(pSeqList obj, size_t pos);

// ==================== 查找 ====================

// 从头到尾查找第一个值等于 x 的元素
// 查找成功时，通过 pos 输出该元素的下标
// 查找失败时，不修改 *pos
// 找到返回 0，未找到或参数无效返回非 0
int SL_Find(const_pSeqList obj, SLDataType x, size_t* pos);

// ==================== 访问与修改 ====================

// 获取下标 pos 处的元素值
// 通过 out 将元素值复制到调用者提供的变量中
// 不返回顺序表内部元素的地址
// pos 的有效范围为 [0, size)
// 成功返回 0，失败返回非 0
int SL_Get(const_pSeqList obj, size_t pos, SLDataType* out);

// 将下标 pos 处的元素值修改为 x
// 不改变顺序表的元素个数和容量
// pos 的有效范围为 [0, size)
// 成功返回 0，失败返回非 0
int SL_Set(pSeqList obj, size_t pos, SLDataType x);

// ==================== 遍历与打印 ====================

// 按下标从小到大的顺序遍历并打印所有有效元素
// 仅负责输出，不修改顺序表内容
void SL_Print(const_pSeqList obj);
