#pragma once

#include <stddef.h>

typedef int SLDataType;

typedef struct SeqList SeqList;

typedef SeqList* pSeqList;
typedef const SeqList* const_pSeqList;

// 创建与销毁
pSeqList SL_Create(void);
void SL_Destroy(pSeqList obj);

// 容量管理
int SL_Reserve(pSeqList obj, size_t capacity);
void SL_Clear(pSeqList obj);
size_t SL_Size(const_pSeqList obj);
size_t SL_Capacity(const_pSeqList obj);
int SL_IsEmpty(const_pSeqList obj);

// 插入
int SL_PushBack(pSeqList obj, SLDataType x);
int SL_PushFront(pSeqList obj, SLDataType x);
int SL_Insert(pSeqList obj, size_t pos, SLDataType x);

// 删除
int SL_PopBack(pSeqList obj);
int SL_PopFront(pSeqList obj);
int SL_Erase(pSeqList obj, size_t pos);

// 查找
int SL_Find(const_pSeqList obj, SLDataType x, size_t *pos);

// 访问与修改
int SL_Get(const_pSeqList obj, size_t pos, SLDataType *out);
int SL_Set(pSeqList obj, size_t pos, SLDataType x);

// 遍历与打印
void SL_Print(const_pSeqList obj);