#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int key = 0;
char buf[100] = {0};

void game(void) {
    int a = 5;
    printf("有一个1~100的整数,你有五次机会可以猜出他.\n");
    while (a) {
        int b = 0;
        printf("请输入要猜的数字:");
        if (fgets(buf, sizeof(buf), stdin) != NULL){
            char* next = NULL;
            long val = strtol(buf, &next, 10);
            if(next != buf){
                b = (int)val;
            }
        }
        else {
            printf("这是不应该存在的分支.");
        }
        if (b < key) {
            printf("猜小了\n");
        } else if (b > key) {
            printf("猜大了\n");
        } else {
            printf("恭喜你!猜对了.\n");
            return;
        }
        a--;
    }
    printf("您的运气挺差的,或许下次可以再试试.\n");
}

int main(void) {
    srand((unsigned)time(NULL));
    int c = 1;
    do {
        c = 1;
        printf("\n");
        printf("================================================\n");
        printf("==================游戏正在准备中==================\n");
        printf("===================0. 退出游戏===================\n");
        printf("===================1. 开始游戏===================\n");
        printf("=====如果你什么都不选,我们将默认你的意图为开始游戏=====\n");
        printf("请选择:");

        // 读整整一行
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            // 如果第一个不是换行， 说明存在某种输入
            if (buf[0] != '\n') {
                char* next = NULL;
                // 以(第三个参数)10进制为准尝试从中提取一个整形,next写入读出整形后的字符串
                long val = strtol(buf, &next, 10);
                if (next != buf) {
                    // 二者若不相等， 说明确实读到数了，返回值有效
                    c = (int)val;
                }
            }
        }

        switch (c) {
            case 0:
                printf("退出中.........................\n");
                break;
            case 1:
                key = rand() % 100 + 1;
                game();
                break;
            default:
                printf("我们只有两个选项,看样子你输错了,奖励你回去重输.");
                break;
        }

    } while (c);
    return 0;
}

/*
 * 【有问题的实现】
 *
 * 这里试图在 fgets() 前通过 getchar() 循环清理 stdin 中残留的字符，
 * 主要是为了处理前面 scanf() 留下的 '\n'。
 *
 * 但这个做法存在一个根本问题：
 * getchar() 并不知道当前 stdin 中是否真的存在“残留输入”。
 *
 * 如果前一次 scanf() 确实留下了 '\n'：
 *     getchar() 会立即读走它，代码表现正常。
 *
 * 如果 stdin 当前没有任何残留字符：
 *     getchar() 会发生阻塞，等待用户输入。
 *     此时用户第一次按下回车，只是让 getchar() 结束；
 *     随后的 fgets() 仍然需要再次等待输入，
 *     因而会出现“需要按两次回车”的现象。
 *
 * 因此：
 *     while ((d = getchar()) != '\n' && d != EOF);
 *
 * 并不是一个通用的“清空输入缓冲区”操作，
 * 它的真实语义是“持续读取 stdin，直到读到换行或 EOF”。
 *
 * 根本原因：
 *     scanf() 与 fgets() 对输入流的消费方式不同，
 *     混合使用时必须手动维护输入流状态，很容易产生残留 '\n'、
 *     意外阻塞等问题。
 *
 * 更合理的方案：
 *     统一使用 fgets() 读取完整的一行，
 *     再使用 strtol() 等函数解析其中的数据。
 */

//void game(void) {
//    int a = 5;
//    printf("有一个1~100的整数,你有五次机会可以猜出他.\n");
//    while (a) {
//        int b = 0;
//        printf("请输入要猜的数字:");
//        while (scanf("%d", &b) != 1);
//        if (b < key) {
//            printf("猜小了\n");
//        } else if (b > key) {
//            printf("猜大了\n");
//        } else {
//            printf("恭喜你!猜对了.\n");
//            return;
//        }
//        a--;
//    }
//    printf("您的运气挺差的,或许下次可以再试试.\n");
//}
//
//int main(void) {
//    srand((unsigned)time(NULL));
//    int c = 1;
//    char buf[100] = {0};
//    do {
//        c = 1;
//        printf("\n");
//        printf("================================================\n");
//        printf("==================游戏正在准备中==================\n");
//        printf("===================0. 退出游戏===================\n");
//        printf("===================1. 开始游戏===================\n");
//        printf("=====如果你什么都不选,我们将默认你的意图为开始游戏=====\n");
//        printf("请选择:");
//
//        // 丢弃当前行剩余内容，包括 '\n'
//        int d;
//        while ((d = getchar()) != '\n' && d != EOF);
//
//        // 读整整一行
//        if (fgets(buf, sizeof(buf), stdin) != NULL) {
//            // 如果第一个不是换行， 说明存在某种输入
//            if (buf[0] != '\n') {
//                char* next = NULL;
//                // 以(第三个参数)10进制为准尝试从中提取一个整形,next写入读出整形后的字符串
//                long val = strtol(buf, &next, 10);
//                if (next != buf) {
//                    // 二者若不相等， 说明确实读到数了，返回值有效
//                    c = (int)val;
//                }
//            }
//        }
//
//        switch (c) {
//            case 0:
//                printf("退出中.........................\n");
//                break;
//            case 1:
//                key = rand() % 100 + 1;
//                game();
//                break;
//            default:
//                printf("我们只有两个选项,看样子你输错了,奖励你回去重输.");
//                break;
//        }
//
//    } while (c);
//    return 0;
//}
