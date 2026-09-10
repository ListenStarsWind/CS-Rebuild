#include <stddef.h>

typedef int data;

typedef struct simV{
    data* _ptr;
    size_t _capa;
    size_t _size;
} simV;

simV* create(size_t n);

void destroy(simV* p);

data* point(simV* p);

int add(data e,simV* p);