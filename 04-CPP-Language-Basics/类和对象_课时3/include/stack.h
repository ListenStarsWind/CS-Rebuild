#pragma once
#include <cstdlib>

namespace wind
{
    typedef int stack_data, * p_stack_data;
    class stack
    {
    public:
        // 成员函数
        stack(size_t sz = 4, p_stack_data pArr = nullptr);
        ~stack();
        stack(const stack& s);
        void Dest();
        bool push(stack_data val);
        bool IsEmpty()const;
        bool ReadTop(stack_data& val)const;
        bool Pop();
        bool ReadPop(stack_data& val);

    private:
        bool Ext();

        // 成员变量
        p_stack_data _p;
        int _top;
        int _capacity;
    };
}


