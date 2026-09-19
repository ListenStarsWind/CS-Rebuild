#include "string_rotation.h"
#include "string_rotate.h"
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

// 初步检查, -1表示绝对没有可能互为旋转字符, 其他数字表示字符串长度
int check(const char* str1, const char* str2){
    int len1 = 0;
    int len2 = 0;
    int count[128] = {0};
    for(; str1[len1] != '\0'; len1++){
        ++count[(uint8_t)str1[len1]];
    }
    for(; str2[len2] != '\0'; len2++){
        --count[(uint8_t)str2[len2]];
    }
    for(int i = 0; i < 128; i++){
        if(count[i] != 0)
            return -1;
    }
    return len1;
}

bool isStringRotationByRepeatedRotation(const char* str1, const char* str2){
    int size = check(str1, str2);
    if(size == -1) return false;
    if(str1[0] == '\0' && str2[0] =='\0') return true;

    char* str3 = (char*)malloc(size+1);
    memset(str3, 0, size+1);
    strcpy(str3, str1);
    for(int i = 0; i < size; i++){
        char* ret = rotateString(str3, size, 1);
        (void)ret;
        if(strcmp(str3, str2) == 0)
            return true;
    }

    return false;
}

bool isStringRotationByConcatenation(const char* str1, const char* str2){
    int size = check(str1, str2);
    if(size == -1) return false;
    if(str1[0] == '\0' && str2[0] =='\0') return true;

    char* str3 = (char*)malloc(2*size+ 1);
    memset(str3, 0, 2*size+1);
    strcat(str3, str1);
    strcat(str3, str1);

    char* ret = strstr(str3, str2);
    free(str3);
    return ret != NULL;
}

bool isStringRotationByCircularIndex(const char* str1, const char* str2){
    int size = check(str1, str2);
    if(size == -1) return false;
    if(str1[0] == '\0' && str2[0] =='\0') return true;

    for(int i = 0; i < size; i++){
        int j = 0;
        for(; j < size; j++){
            if(str2[j] != str1[(i+j)%size])
                break;
        }
        if(j == size)
            return true;
    }
    return false;
}
