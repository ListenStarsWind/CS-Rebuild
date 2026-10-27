#include "stack.h"

#include <cstdio>

wind::stack::stack(size_t sz, p_stack_data pArr) {
    if (pArr == nullptr) {
        auto p = (p_stack_data)realloc(pArr, sizeof(stack_data) * sz);
        if (p == nullptr) {
            perror("Error: build fail!");
        }
        _p = p;
        _top = 0;
        _capacity = sz;
    } else {
        _p = pArr;
        _top = sz;
        _capacity = sz;
    }
}

wind::stack::stack(const stack& s){
    auto p = (p_stack_data)std::realloc(nullptr, sizeof(stack_data)*s._capacity);
    if(p == nullptr){
        perror("Error: build fail!");
    }
    for(int i = 0; i < s._top; i++){
        p[i] = s._p[i];
    }
    _p = p;
    _top = s._top;
    _capacity = s._capacity;
}

wind::stack::~stack(){
    std::free(_p);
    _top = _capacity = 0;
}

void wind::stack::Dest() {
    std::free(_p);
    _p = nullptr;
    _top = _capacity = 0;
}

bool wind::stack::Ext() {
    int newCapacity = _capacity == 0 ? 2 : _capacity * 2;
    p_stack_data p = (p_stack_data)std::realloc(_p, sizeof(stack_data) * newCapacity);
    if (p == nullptr) {
        std::perror("stack.Ext realloc fail");
        return false;
    }
    _p = p;
    _capacity = newCapacity;
    return true;
}

bool wind::stack::push(stack_data val) {
    if (_top == _capacity && !Ext()) {
        return false;
    }
    _p[_top++] = val;
    return true;
}

bool wind::stack::IsEmpty() const {
    if (_top == 0)
        return true;
    else
        return false;
}

bool wind::stack::ReadTop(stack_data& val) const {
    if (IsEmpty())
        return false;
    else {
        val = _p[_top - 1];
        return true;
    }
}

bool wind::stack::Pop() {
    if (IsEmpty())
        return false;
    else {
        _top--;
        return true;
    }
}

bool wind::stack::ReadPop(stack_data& val) {
    if (IsEmpty())
        return false;
    else {
        ReadTop(val);
        return Pop();
    }
}
