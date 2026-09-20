#include "search_in_sorted_matrix.h"

bool searchInSortedMatrix(int m, int n, int (*arr)[n], int k) {
    int i = 0, j = n - 1;
    while (i < m && j >= 0) {
        if (arr[i][j] > k)
            j--;
        else if (arr[i][j] < k)
            i++;
        else
            return true;
    }
    return false;
}