#pragma once

#include <stddef.h>

// 从 src 尽可能复制 n 个字符到 dest, 如果 src 过短, 剩下的用 '\0'填充
// 如果太长, dest 就会只复制到 第 n 个字符, 而不追加 '\0'
char* myStrncpy(char* dest, const char* src, size_t size);