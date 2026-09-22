#pragma once
#include <stddef.h>

void myQsort(void* base, size_t count, size_t size, int (*compare)(const void*, const void*));