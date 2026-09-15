#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

// 寻找二进制中1的数目
int count_ones_by_clearing(uint64_t n) {
    int ret = 0;
    while (n) {
        ret++;
        n &= n - 1;
    }
    return ret;
}

int count_different_bits(uint64_t m, uint64_t n) {
    uint64_t tmp = m ^ n;
    return count_ones_by_clearing(tmp);
}

int main(void) {
    uint64_t arr1[10] = {0};
    uint64_t arr2[10] = {0};

    // scanf 格式串中的空白字符会跳过输入中连续的 空白字符
    // 即 isspace() 所识别的字符，如 ' '、'\n'、'\t'、'\r'、'\v'、'\f'。
    for (int i = 0; i < 10; i++) scanf(" (%" SCNu64 ",%" SCNu64 ")", arr1 + i, arr2 + i);

    for (int i = 0; i < 10; i++) printf("%d ", count_different_bits(arr1[i], arr2[i]));

    printf("\n");

    return 0;
}
