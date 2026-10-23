#include "DoublyLinkedList.h"
#include <cstdlib>
#include <new>
#include <boost/assert.hpp>

struct DLLNode{
    DLLDataType _data;
    DLLNode* _prev;
    DLLNode* _next;
};

typedef DLLNode SentinelNode;
typedef DLLNode 

static DLLNode* Create_Node(void){
    void* memory = std::malloc(sizeof(DLLNode));
    if(memory == nullptr) return nullptr;

    DLLNode* ptr = nullptr;

    try {
        ptr =::new(memory) DLLNode{DLLDataType(), nullptr, nullptr};

    } catch (...) {
        std::free(memory);
        return nullptr;
    }
    return ptr;
}

static int Destroy_Node(DLLNode* node){
    if(node != nullptr){
        node->~DLLNode();
        std::free(node);   
    }
    return 0;
}

static inline void Set_Node_Data(DLLNode* node, DLLDataType x){
    BOOST_ASSERT_MSG(node != nullptr, "Invalid node pointer");
    node->_data = x;
}

static inline void Set_Node_Prev(DLLNode* node, DLLNode* prev){
    BOOST_ASSERT_MSG(node != nullptr, "Invalid node pointer");
    node->_prev = prev;
}

static inline void Set_Node_Next(DLLNode* node, DLLNode* next){
    BOOST_ASSERT_MSG(node != nullptr, "Invalid node pointer");
    node->_next = next;
}

static inline DLLDataType Get_Node_Data(const DLLNode* node){
    BOOST_ASSERT_MSG(node != nullptr, "Invalid node pointer");
    return node->_data;
}

static inline DLLNode* Get_Node_Prev(const DLLNode* node){
    BOOST_ASSERT_MSG(node != nullptr, "Invalid node pointer");
    return node->_prev;
}

static inline DLLNode* Get_Node_Next(const DLLNode* node){
    BOOST_ASSERT_MSG(node != nullptr, "Invalid node pointer");
    return node->_next;
}

DoublyLinkedList* DLL_Create(void){
    auto sentinel = Create_Node();
    Set_Node_Prev(sentinel, sentinel);
    Set_Node_Next(sentinel, sentinel);
    return sentinel;
}

void DLL_Destroy(DoublyLinkedList* list){

}

size_t DLL_Size(const DoublyLinkedList* list);

bool DLL_IsEmpty(const DoublyLinkedList* list);

DLLNode* DLL_Front(DoublyLinkedList* list);

DLLNode* DLL_Back(DoublyLinkedList* list);

DLLNode* DLL_Next(DoublyLinkedList* list, DLLNode* node);

DLLNode* DLL_Prev(DoublyLinkedList* list, DLLNode* node);

DLLNode* DLL_PushFront(DoublyLinkedList* list, DLLDataType value);

DLLNode* DLL_PushBack(DoublyLinkedList* list, DLLDataType value);

DLLNode* DLL_InsertBefore(DoublyLinkedList* list, DLLNode* position, DLLDataType value);

DLLNode* DLL_InsertAfter(DoublyLinkedList* list, DLLNode* position, DLLDataType value);

void DLL_PopFront(DoublyLinkedList* list);

void DLL_PopBack(DoublyLinkedList* list);

void DLL_Erase(DoublyLinkedList* list, DLLNode* node);

void DLL_Clear(DoublyLinkedList* list){
    BOOST_ASSERT_MSG(list != nullptr, "Invalid node pointer");
    DLLNode* sentinel = list;
    DLLNode* curr = sentinel;
    while(next != sentinel){

    }
}

DLLNode* DLL_Find(DoublyLinkedList* list, DLLDataType value);

DLLDataType DLL_Get(const DoublyLinkedList* list, const DLLNode* node);

void DLL_Set(DoublyLinkedList* list, DLLNode* node, DLLDataType value);
