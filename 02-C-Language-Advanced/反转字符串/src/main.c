#include <string.h>
#include <stdio.h>

void reverseString(char* s, int sSize) {
    int left = 0;
    int right = sSize-1;
    while(left < right){
        char tmp = s[left];
        s[left++] = s[right];
        s[right--] = tmp;
    }
}


// fgets 一次读取通常会因为以下三种情况结束：
//
// 1. 读到换行符 '\n'
//    如果缓冲区容量足够，'\n' 本身也会被读入数组。
//
// 2. 已经读取了 size - 1 个字符
//    fgets 必须给字符串结尾的 '\0' 留一个位置，因此最多读取 size - 1 个字符。
//    剩余字符仍然留在输入流中，等待下一次读取。
//
// 3. 遇到 EOF
//    EOF 并不是一个实际字符，而表示输入流已经没有更多数据。
//
//    如果遇到 EOF 之前，本次 fgets 已经读到了一些字符，
//    那么这些字符仍然会作为一个有效字符串返回。
//
//    如果本次调用一个字符都没有读到就遇到 EOF，
//    fgets 才会返回 NULL。
//
// 在 Linux 终端的规范模式下，Ctrl+D 是 TTY 的 VEOF 控制字符。
// 它不是向进程发送信号，也不是把一个“EOF 字符”塞进输入流。
// 当当前行没有待提交字符时，Ctrl+D 可以使底层 read() 返回 0，
// 从而被 stdio 层解释为 EOF。
// 如果 Ctrl+D 前已经输入了一些字符，则通常先把这些字符交给程序。
//
// fgets 返回 NULL 还可能意味着发生了底层 I/O 错误。
// 如果确实需要区分，可以使用 feof() 和 ferror()。
//
// fgets 相比已经废弃的 gets()，最重要的改进是可以指定缓冲区容量，
// 从而限制一次最多读入多少字符。
// 另外，fgets 在读到换行时会把 '\n' 一并保存到数组中。
//
// 因此，单独连续使用 fgets 时，不存在 scanf("%d") 那种
// “转换结束后把换行符遗留在输入流里，导致下一次整行读取立刻结束”的典型问题。
//
// 但如果 scanf 和 fgets 混用，就必须明确 scanf 到底消费到了哪里。
//
// 例如：
//     scanf("%d", &n);
//
// %d 会跳过前导空白并读取数字，但数字后面的 '\n' 通常仍留在输入流中。
// 如果紧接着调用 fgets()，它会立即读到这个 '\n'，
// 得到一个只包含 "\n" 的字符串，看起来就像 fgets 被“跳过”了一样。
//
// 因此不要无条件地写所谓“清空输入缓冲区”的循环。
// 如果输入流中本来没有需要丢弃的数据，这种循环反而会阻塞等待用户输入，
// 甚至把下一批真正有效的输入当成垃圾吃掉。
//
// 与其事后到处“清缓冲区”，更好的思路是统一输入模型。
// 对普通文本输入，通常可以优先使用 fgets() 读取整行，
// 再使用 sscanf()、strtol() 等函数解析这一行。
//
// 可参考同仓库下“猜数字”示例，那里曾经出现过类似的输入残留问题。

int main(void){
    char s[1000] = {'\0'};
    int capa = sizeof(s) - 1;
    while(fgets(s, capa, stdin) != NULL){
        int size = strlen(s);
        if(s[size-1] == '\n'){
            s[size-1] = '\0';
            size--;
        }
        reverseString(s, size);
        printf("%s\n", s);
    }

    return 0;    
}
