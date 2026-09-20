#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "search_in_sorted_matrix.h"

int main(void) {
    int m = 0, n = 0;
    char buf[64] = {0};
    if (fgets(buf, 63, stdin)) {
        int sz = strlen(buf);
        if (buf[sz - 1] == '\n') buf[--sz] = '\0';
        char *next1 = NULL, *next2 = NULL;
        long val = strtol(buf, &next1, 10);
        if (next1 != buf) {
            m = (int)val;
            val = strtol(next1, &next2, 10);
            if (next1 != next2) {
                n = (int)val;
                int(*arr)[n] = (int(*)[n])malloc(sizeof * arr * m);
                if (arr != NULL) {
                    for (int i = 0; i < m; i++) {
                        if (fgets(buf, 63, stdin)) {
                            char* curr = buf;
                            for (int j = 0; j < n; j++) {
                                char* next = NULL;
                                val = strtol(curr, &next, 10);
                                if (next != curr) {
                                    arr[i][j] = (int)val;
                                    curr = next;
                                    next = NULL;
                                }
                            }
                        }
                    }
              
                    int k = 0;
                    while (fgets(buf, 63, stdin) != NULL) {
                        int sz = strlen(buf);
                        if (buf[sz - 1] == '\n') buf[--sz] = '\0';
                        char* next = NULL;
                        val = strtol(buf, &next, 10);
                        if (next != buf) k = (int)val;
                        if (searchInSortedMatrix(m, n, arr, k))
                            printf("矩阵中存在数字%d\n", k);
                        else
                            printf("矩阵中不存在目标数字\n");
                    }
                    free(arr);
                }
            }
        }
        return 0;
    }
}