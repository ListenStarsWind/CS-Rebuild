#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* rotateLeftByRepeatedShift(char* str, int size, int k) {
    k = k % size;
    if (k == 0) return str;

    for (int i = 0; i < k; i++) {
        char tmp = str[0];
        for (int j = 0; j < size - 1; j++) {
            str[j] = str[j + 1];
        }
        str[size - 1] = tmp;
    }
    return str;
}

char* rotateLeftByBuffer(char* str, int size, int k) {
    if (size > 512) return NULL;

    k = k % size;
    if (k == 0) return str;

    char tmp[512] = {0};
    memcpy(tmp, str, k);
    memmove(str, str + k, size - k);
    memcpy(str + size - k, tmp, k);
    return str;
}

char* reverseRange(char* str, int left, int right) {
    while (left < right) {
        char tmp = str[left];
        str[left++] = str[right];
        str[right--] = tmp;
    }
    return str;
}

char* rotateLeftByReversal(char* str, int size, int k) {
    k = k % size;
    if (k == 0) return str;

    reverseRange(str, 0, k - 1);
    reverseRange(str, k, size - 1);
    reverseRange(str, 0, size - 1);
    return str;
}

char* _rotateLeftByBlockSwap(char* str, char* tmp, int start1, int start2, int size) {
    (void)start1;
    int len1 = start2;
    int len2 = size - len1;
    if (len1 == len2) {
        memcpy(tmp, str, len1);
        memcpy(str, str + start2, len2);
        memcpy(str + start2, tmp, len1);
    }
    if (len1 < len2) {
        memcpy(tmp, str, len1);
        memcpy(str, str + start2, len1);
        memcpy(str + start2, tmp, len1);
        _rotateLeftByBlockSwap(str + len1, tmp, 0, len1, size - len1);
    }
    if (len1 > len2) {
        memcpy(tmp, str + start2, len2);
        memcpy(str + start2, str + len1 - len2, len2);
        memcpy(str + len1 - len2, tmp, len2);
        _rotateLeftByBlockSwap(str, tmp, 0, len1 - len2, size - len2);
    }
    return str;
}

char* rotateLeftByBlockSwap(char* str, int size, int k) {
    if (size > 512) return NULL;

    k = k % size;
    if (k == 0) return str;
    char tmp[512] = {0};
    return _rotateLeftByBlockSwap(str, tmp, 0, k, size);
}

int greatestCommonDivisor(int x, int y) {
    int z = 0;
    while ((z = x % y)) {
        x = y;
        y = z;
    }
    return y;
}

char* rotateLeftByCycles(char* str, int size, int k) {
    k = k % size;
    if (k == 0) return str;

    int gcd = greatestCommonDivisor(size, k);
    int len = size / gcd;
    for (int i = 0; i < gcd; i++) {
        char tmp = str[i];
        int j = i;
        for (int z = 1; z < len; z++) {
            int next = (i + z * k) % size;
            str[j] = str[next];
            j = next;
        }
        str[j] = tmp;
    }
    return str;
}

char* start(char* buf) {
    printf("接下来你可以不断输入长度小于511的字符串, 若想退出循环, 使用 CTRL D\n");
    return fgets(buf, 512, stdin);
}

int main(void) {
    char* (*fp[5])(char*, int, int) = {rotateLeftByRepeatedShift, rotateLeftByBuffer,
                                       rotateLeftByReversal, rotateLeftByBlockSwap,
                                       rotateLeftByCycles};
    char buf1[512] = {0};
    char buf2[11] = {0};
    char buf3[512] = {0};
    while (start(buf1) != NULL) {
        int size = strlen(buf1);
        if (buf1[size - 1] == '\n') {
            buf1[size - 1] = '\0';
        }
        printf("接下来请输入左旋参数(k): ");
        int k = -1;
        do {
            fgets(buf2, 10, stdin);
            char* next = NULL;
            int val = (int)strtol(buf2, &next, 10);
            if (next != buf2) {
                k = val;
            }
        } while (k == -1);
        for (int i = 0; i < 5; i++) {
            memcpy(buf3, buf1, 512);
            printf("第%d个方法结果: %s\n", i + 1, fp[i](buf3, strlen(buf3), k));
        }
        printf("\n");
    }
    return 0;
}
