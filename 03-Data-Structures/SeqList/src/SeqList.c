#include "SeqList.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


struct SeqList {
    SLDataType* _arr;
    size_t _size;
    size_t _capacity;
};

pSeqList SL_Create(void) {
    pSeqList p = malloc(sizeof(SeqList));
    if (p == NULL) return NULL;
    SLDataType* data = realloc(NULL, sizeof(SLDataType)*4);
    if(data == NULL){
        free(p);
        return NULL;
    }
    p->_arr = data;
    p->_size = 0;
    p->_capacity = 4;
    return p;
}

void SL_Destroy(pSeqList obj) {
    free(obj->_arr);
    free(obj);
}

int SL_Reserve(pSeqList obj, size_t capacity) {
    if(capacity <= obj->_capacity) return 0;

    size_t growthCapacity = obj->_capacity + obj->_capacity / 2; 
    size_t newCapacity = capacity > growthCapacity ? capacity : growthCapacity;

    SLDataType* data = realloc(obj->_arr, sizeof(SLDataType)* newCapacity);
    if(data == NULL) return errno;
    obj->_arr = data;
    obj->_capacity = newCapacity;
    return 0;
}

void SL_Clear(pSeqList obj) {
    obj->_size = 0;
}

size_t SL_Size(const_pSeqList obj) {
    return obj->_size;
}

size_t SL_Capacity(const_pSeqList obj) {
    return obj->_capacity;
}

int SL_IsEmpty(const_pSeqList obj) {
    return SL_Size(obj) == 0;
}

int SL_PushBack(pSeqList obj, SLDataType x) {
    return SL_Insert(obj, SL_Size(obj), x);
}

int SL_PushFront(pSeqList obj, SLDataType x) {
    return SL_Insert(obj, 0, x);
}

int SL_Insert(pSeqList obj, size_t pos, SLDataType x) {
    if(pos < 0 || pos > obj->_size) return ERANGE;
    if (obj->_size == obj->_capacity) {
        int ret = SL_Reserve(obj, obj->_size+1);
        if(ret != 0) return ret;
    }
    
    memmove(obj->_arr + pos + 1, obj->_arr + pos,
            (obj->_size - pos) * sizeof(SLDataType));
    obj->_size++;
    obj->_arr[pos] = x;
    return 0;
}

int SL_PopBack(pSeqList obj) {
    return SL_Erase(obj, SL_Size(obj) - 1);
}

int SL_PopFront(pSeqList obj) {
    return SL_Erase(obj, 0);
}

int SL_Erase(pSeqList obj, size_t pos) {
    if (pos < 0 || pos >= obj->_size) return ERANGE;
    memmove(obj->_arr + pos, obj->_arr + pos + 1,
            (obj->_size - pos - 1) * sizeof(SLDataType));
    obj->_size--;
    return 0;
}

int SL_Find(const_pSeqList obj, SLDataType x, size_t* pos) {
    int size = obj->_size;
    for (int i = 0; i < size; i++) {
        if (obj->_arr[i] == x) {
            *pos = i;
            return 0;
        }
    }
    return ENOENT;
}

int SL_Get(const_pSeqList obj, size_t pos, SLDataType* out) {
    if (pos < 0 || pos >= obj->_size) return ENOENT;
    *out = obj->_arr[pos];
    return 0;
}

int SL_Set(pSeqList obj, size_t pos, SLDataType x) {
    if (pos < 0 || pos >= obj->_size) return ENOENT;
    obj->_arr[pos] = x;
    return 0;
}

void SL_Print(const_pSeqList obj) {
    int size = obj->_size;
    for (int i = 0; i < size; i++) {
        printf("%d ", obj->_arr[i]);
    }
    printf("\n");
}