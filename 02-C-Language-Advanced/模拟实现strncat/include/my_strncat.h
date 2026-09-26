#pragma once

#include <stddef.h>

// strncat 将最多 size 个有效字符拼接到 dst 已有字符串之后, 并且
// 无论有效字符个数是等于还是小于 size, 都会再在气候追加 '\0'
char* myStrncat(char* dst, const char* src, size_t size);