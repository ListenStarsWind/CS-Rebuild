#include "SeqList.h"
#include <stdlib.h>
#include <string.h>

struct SeqList{
    SLDataType* _arr;
    size_t _size;
    size_t _capacity;
};

// 创建与销毁
pSeqList SL_Create(void){
    pSeqList p = malloc(sizeof(SeqList));
    if(p == NULL) return NULL;
    p->_arr = NULL;
    p->_size = 0;
    p->_capacity = 0;
}

void SL_Destroy(pSeqList obj){
    free(obj);
}

// 容量管理
int SL_Reserve(pSeqList obj, size_t capacity){
    SLDataType* data = malloc(sizeof(SLDataType)*capacity);
    if(data == NULL) return 0;
    p->_arr = data;
    p->_capacity = capacity;
    return capacity;
}

void SL_Clear(pSeqList obj){
    p->_size = 0;
}

size_t SL_Size(const_pSeqList obj){
    return obj->_size; 
}
size_t SL_Capacity(const_pSeqList obj){
    return obj->_capacity;
}
int SL_IsEmpty(const_pSeqList obj){
    return SL_Size(obj) == 0;
}

// 插入
int SL_PushBack(pSeqList obj, SLDataType x){
    return SL_Insert(obj, SL_Size(obj), x);
}

int SL_PushFront(pSeqList obj, SLDataType x){
    return SL_Insert(obj, 0, x);
}
int SL_Insert(pSeqList obj, size_t pos, SLDataType x){
    if(obj->_size == obj->_capacity) return 0;
    if(pos < 0 || pos > obj->_size) return 0;
    memmove((unsigned char*)obj->_arr + pos, (unsigned char*)obj->_arr+pos+1, (obj->_size-pos)*sizeof(SLDataType));
    obj->_size++;
    obj->_arr[pos] = x;
    return 1;
}

// 删除
int SL_PopBack(pSeqList obj){
    return SL_Erase(obj, SL_Size(obj)-1);
}
int SL_PopFront(pSeqList obj){
    return SL_Erase(obj, 0);
}
int SL_Erase(pSeqList obj, size_t pos){
    if(pos < 0 || pos >= obj->_size) return 0;
    memmove((unsigned char*)obj->_size+pos+1, (unsigned char*)obj->_size+pos, (obj->_size - pos) * sizeof(SLDataType));
    obj->_size--;
    return 1;
}

// 查找
int SL_Find(const_pSeqList obj, SLDataType x, size_t *pos);

// 访问与修改
int SL_Get(const_pSeqList obj, size_t pos, SLDataType *out);
int SL_Set(pSeqList obj, size_t pos, SLDataType x);

// 遍历与打印
void SL_Print(const_pSeqList obj);