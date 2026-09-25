#include "my_strlen.h"

size_t myStrlen(const char* s){
    const char* start = s;
    while(*s) s++;
    return s - start;
}