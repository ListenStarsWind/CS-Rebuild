#include "simVector.h"
#include <stdlib.h>

simV* create(size_t n){
    simV* p = malloc(sizeof(simV));
    p->_ptr = malloc(sizeof(data) * (n+1));
    p->_capa = n;
    p->_size = 0;
    return p;
}

void destroy(simV* p){
    free(p->_ptr);
    free(p);
}

data* point(simV* p){
    return p->_ptr + 1;
}

int add(data e, simV* p){
    if(p->_size >= p->_capa)
        return -1;
    p->_size++;
    size_t i = 1;
    for(; i < p->_size; i++){
        if(e < p->_ptr[i])
        {
            size_t end = i;
            size_t cur = p->_size - 1;
            for(; cur >= end; cur--){
                p->_ptr[cur + 1] = p->_ptr[cur];
            }
            break;
        }
    }
    p->_ptr[i] = e;
    return 1;
}