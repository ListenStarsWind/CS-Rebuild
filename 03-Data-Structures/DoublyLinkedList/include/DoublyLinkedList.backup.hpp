#pragma once

namespace wind {
class NodeBase {
   private:
    NodeBase* _prev;
    NodeBase* _next;

    typedef NodeBase* pNodeBase;
    typedef pNodeBase& rpNodeBase;
    typedef pNodeBase const& const_rpNodeBase;

   public:
    NodeBase(void) : _prev(nullptr), _next(nullptr) {};
    NodeBase(NodeBase* prev, NodeBase* next) : _prev(prev), _next(next) {};
    rpNodeBase prev() {
        return _prev;
    };
    rpNodeBase next() {
        return _next;
    };
    const_rpNodeBase prev() const {
        return _prev;
    };
    const_rpNodeBase next() const {
        return _next;
    };
};


template<typename T>
class DataNode: protected NodeBase{
    private:
    T _data;

    public:

    
};
}  // namespace wind
