#include <stdio.h>

#include "SeqList.h"
#include "system_info.h"

int main(void) {
    printSystemInfo();
    pSeqList sl = SL_Create();
    if (sl == NULL) return 1;

    // 尾插
    for (int i = 1; i <= 5; ++i) SL_PushBack(sl, i * 10);

    SL_Print(sl);

    // 头插、指定位置插入
    SL_PushFront(sl, 100);
    SL_Print(sl);
    SL_Insert(sl, 2, 200);
    SL_Print(sl);

    // 查找
    size_t pos;
    if (SL_Find(sl, 200, &pos) == 0) printf("找到 200, 位置:%zu\n", pos);

    // 访问、修改
    SLDataType x;
    if (SL_Get(sl, 2, &x) == 0) printf("位置 2 的值：%d\n", x);

    SL_Set(sl, 2, 999);
    SL_Print(sl);

    // 删除
    SL_PopFront(sl);
    SL_PopBack(sl);
    SL_Erase(sl, 1);
    SL_Print(sl);

    // 容量管理
    SL_Reserve(sl, 100);
    printf("size = %zu, capacity = %zu\n", SL_Size(sl), SL_Capacity(sl));

    // 清空
    SL_Clear(sl);
    printf("清空后: size = %zu, empty = %d\n", SL_Size(sl), SL_IsEmpty(sl));

    SL_Destroy(sl);
    return 0;
}
