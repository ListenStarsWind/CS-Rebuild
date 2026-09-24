#include <stdio.h>
#include <string.h>

#include "system_info.h"

void test1(void) {
  int a[] = {1, 2, 3, 4};
  printf("%zu\n", sizeof(a));
  printf("%zu\n", sizeof(a + 0));
  printf("%zu\n", sizeof(*a));
  printf("%zu\n", sizeof(a + 1));
  printf("%zu\n", sizeof(a[1]));
  printf("%zu\n", sizeof(&a));
  printf("%zu\n", sizeof(*&a));
  printf("%zu\n", sizeof(&a + 1));
  printf("%zu\n", sizeof(&a[0]));
  printf("%zu\n", sizeof(&a[0] + 1));
}

// 在  C语言中, 数组名通常被认为是指向数组首元素的指针, 除非 不加任何处理直接交给  sizeof  或者  &
// sizeof(a) [另一种写法是 sizoef*对象名, 如 sizeof*a]   中的a 是一个数组  类型为 int [4], 因此,大小为16 字节
// sizeof(a + 0) a 不是直接交给 sizeof 而是加了 0,  所以此处 a 被视为 首元素指针,加了0之后依旧是首元素指针, 类型 int*, 8
// sizeof(*a) a 不是直接交给 sizeof 而是进行了解引用,所以此处 a 被视为 首元素指针, 解引用后被视为首元素, int , 4 字节
// sizeof(a + 1) a 不是直接交给sizeof , 而是加了1, 这意味着此处的 a 表示的是 首元素指针, 在被 加1 之后, 表示指向第二元素指针,int* 8
// sizeof(a[1]) 表示第二个元素本身, int 4 sizeof(&a) 此处  a 表示整个数组,&a表示一个指向整个数组的指针, int (*)[4], 8字节
// sizeof(*&a) 一种说法是  * 和  & 抵消, 等效于sizeof(a), 另一种说法是 &a 是整个数组指针, 解引用后还是数组, 不管怎样, 类型是 int [4], 16
// sizeof(&a + 1) &a 是整个数组的指针, int (*) [4], 再加一就变成跳过 int [4] 的长度, 但还是  int(*)[4], 8,另外, sizeof 在编译阶段会把参数换成类型, 所以表达式实际运行不计算, 也就不会段错误
// sizeof(&a[0])  首元素取出(a[0])后再取地址, 依旧是  int*, 8
// sizeof(&a[0] + 1) 指向第二元素的指针, 依旧是  int*, 8;

void test2(void) {
  char arr[] = {'a', 'b', 'c', 'd', 'e', 'f'};
  printf("%zu\n", sizeof(arr));
  printf("%zu\n", sizeof(arr + 0));
  printf("%zu\n", sizeof(*arr));
  printf("%zu\n", sizeof(arr[1]));
  printf("%zu\n", sizeof(&arr));
  printf("%zu\n", sizeof(&arr + 1));
  printf("%zu\n", sizeof(&arr[0] + 1));
}
// sizeof(arr) arr 直接交给 sizeof , 所以表示整个数组 , 即 char [6], 6 字节
// sizeof(arr + 0) arr 经过加 0 之后再交给 sizeof, 所以 arr 代表首元素指针, char*, 8
// sizeof(*arr) 解引用首元素,  char, 1 字节
// sizeof(arr[1]) 解引用第二个元素 , char, 1 字节
// sizeof(&arr) 此为整个数组的指针, 即 char (*)[6], 8字节
// sizeof(&arr + 1) 次为整个数组的指针 , char (*)[6], 加1后跳过 char[6], 不过类型依旧不变, 为 8
// sizeof(&arr[0] + 1) 指向第二元素的指针, char* 8字节

void test3(void) {
  char arr[] = {'a', 'b', 'c', 'd', 'e', 'f'};
  printf("%zu\n", strlen(arr));
  printf("%zu\n", strlen(arr + 0));
  // printf("%zu\n", strlen(*arr));
  // printf("%zu\n", strlen(arr[1]));
  printf("%zu\n", strlen(&arr));
  printf("%zu\n", strlen(&arr + 1));
  printf("%zu\n", strlen(&arr[0] + 1));
}
//  strlen(arr) 没加 '\0', 而 strlen 读到 '\0'才停止, 所以答案由具体运行情况决定(多少之后会出现
//  '\0')
//  strlen(arr + 0) 这也是首元素, 和上面一样, 因此还是由具体运行情况决定
//  strlen(*arr)解引用出第一个元素 , 第一个元素是 char , char 的  ASCII 码被理解为地址, strlen
//  会查询进程无权管理的空间, 进而段错误, 如果编译选项更加严格,整形向地址的隐式转变将会被视为语法错误
//  strlen(arr[1]) 也是解引用, 也是  ASCII 被理解为地址,极大概率引发段错误, 同样, 编译选项严格情况, 被视为 error , 无法通过编译
//  strlen(&arr)  &arr得到的是数组指针, 数组指针被强制类型转换, 理解为  char*, 还是首元素地址, 依旧由实际运行情况决定
//  strlen(&arr + 1) 数组指针加1后跳过 char [6], 其后被理解为  char* , 依旧由具体运行情况决定
//  strlen(&arr[0] + 1) 第二个元素地址, 相比 首元素开始, 只不过少了一个长度,不过还是由具体运行情况决定

void test4(void) {
  char arr[] = "abcdef";
  printf("%zu\n", sizeof(arr));
  printf("%zu\n", sizeof(arr + 0));
  printf("%zu\n", sizeof(*arr));
  printf("%zu\n", sizeof(arr[1]));
  printf("%zu\n", sizeof(&arr));
  printf("%zu\n", sizeof(&arr + 1));
  printf("%zu\n", sizeof(&arr[0] + 1));
}
//  sizeof(arr) arr 在此处表示具体数组, C中字符串末尾会添加终止符'\0', 所以是7个元素, 7个字节
//  sizeof(arr + 0) 这里是指向第一个元素的指针, char* 8字节
//  sizeof(*arr)  对首元素的解引用 , char, 1字节
//  sizeof(arr[1])  对于第二个元素的解引用, char, 1字节
//  sizeof(&arr) 这是指向整个数组的指针, char(*)[7], 8字节
//  sizeof(&arr + 1) 与上面相同, 这依旧是指向数组的指针, 加1使他跳过了char[7]长度, 但类型不变, 8字节
//  sizeof(&arr[0] + 1) 表示指向第二个元素的地址, 类型是 char*, 8 字节

void test5(void) {
  char arr[] = "abcdef";
  printf("%zu\n", strlen(arr));
  printf("%zu\n", strlen(arr + 0));
  // printf("%zu\n", strlen(*arr));
  // printf("%zu\n", strlen(arr[1]));
  printf("%zu\n", strlen(&arr));
  printf("%zu\n", strlen(&arr + 1));
  printf("%zu\n", strlen(&arr[0] + 1));
}
//  strlen(arr) 相比数组, 看作字符串显然更加合适, 这里为字符串长度, 6字节
//  strlen(arr + 0) 这里还是被视为首元素地址, 依旧是长度 6 字节
//  strlen(*arr)  这里  ASCII 将会被尝试解释为 char* , 但和上面差不多, 会越权访问
//  strlen(arr[1])   和上述同理
//  strlen(&arr) &arr将会被理解为整个数组的指针, 类型为 char (*)[7], 他能不能被转换成 const char* ,
//  这不清楚, 但如果成功转换, 那也是字符串长度 strlen(&arr + 1)  这样会直接越过字符串, 来到
//  '\0'后的元素, 是越界访问 strlen(&arr[0] + 1) 从第二个元素开始, 所以长度会少一个字节

void test6(void) {
  char* p = "abcdef";
  printf("%zu\n", sizeof(p));
  printf("%zu\n", sizeof(p + 1));
  printf("%zu\n", sizeof(*p));
  printf("%zu\n", sizeof(p[0]));
  printf("%zu\n", sizeof(&p));
  printf("%zu\n", sizeof(&p + 1));
  printf("%zu\n", sizeof(&p[0] + 1));
}
//  sizeof(p) 对象p是一个char* , 也就是指针, 8字节
//  sizeof(p + 1) 参数是指向字符串第二个字符的指针， char*, 8
//  sizeof(*p)  解引用得到 char 1字节
//  sizeof(p[0]) 换种方法的解引用, 最后得到的依旧是 char , 1
//  sizeof(&p)  p 不是数组, 取地址后是 char**, 8字节
//  sizeof(&p + 1) &p 得到的是 局部对象 p 的地址, 再加1跨过 char* , 具体指向什么需要探讨
//  函数栈帧中的变量布局, 不过还是 char**, 8 字节 sizeof(&p[0] + 1)  得到指向第二个字符的指针, char*
//  , 8 字节

void test7(void) {
  printf("系统字节序: ");
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  printf("小端\n");
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  printf("大端\n");
#endif
  char* p = "abcdefg";
  char bytes[8] = {0};
  for (int i = 0; i < 8; i++) {
    bytes[i] = ((unsigned char*)&p)[7 - i];
  }
  printf("&p = %p\n", (void*)&p);
  printf("p  = %p\n", (void*)p);
  printf("%zu\n", strlen(p));
  printf("%zu\n", strlen(p + 1));
  // printf("%zu\n", strlen(*p));
  // printf("%zu\n", strlen(p[0]));
  printf("%zu\n", strlen(&p));
  printf("%zu\n", strlen(bytes));
  printf("%zu\n", strlen(&p + 1));
  printf("%zu\n", strlen(&p[0] + 1));
}
//  strlen(p) 就是整个字符串的长度  7 字节
//  strlen(p + 1）， 从第二个字符开始， 少了一字节， 6
//  strlen(*p)  编译器不允许  ASCII 码转成 char*, 即使允许， 也大概率越权访问
//  strlen(p[0])， 同上
//  strlen(&p)  char** 会被 强转回 char* , 但这个 char* 指向的仍旧是  p 自己, 而非字符串,
//  字符串"abcdef"存在静态区, 所以结果取决于函数栈帧变量布局
//  strlen(&p + 1)  同上 strlen(&p[0] + 1) 从第二个字符开始, 少了一个字符  6
// 注: 不要用长度为6的字符串进行试验!
// 这是因为用户态代码的地址空间通常只允许使用低48位(也就是低6字节)。
// 因此P中的值，也就是指向静态区常量字符串地址的高两字节有效位是0。
// 而现在的主流机器都是小端，数值有效的低位是放在低地址处的。
// 所以当我们将P视为有八个元素的字符字符串时，最后两字节都是0，所以长度最大也就是6。
// 当然也有可能低6的有效字节中恰好有一个全为0，这样就会比6更小一些，不过这种情况概率比较小。
// 如果换成大端，这里我们通过倒着初始化数组模拟出了大端情况。
// 此时，仍旧是从低地址元素向高地址元素读，但由于低地址元素存的是高有效位，
// 所以一下子就读到0，长度就为0。

void test8(void) {
  int a[3][4] = {0};
  printf("%zu\n", sizeof(a));
  printf("%zu\n", sizeof(a[0][0]));
  printf("%zu\n", sizeof(a[0]));
  printf("%zu\n", sizeof(a[0] + 1));
  printf("%zu\n", sizeof(*(a[0] + 1)));
  printf("%zu\n", sizeof(a + 1));
  printf("%zu\n", sizeof(*(a + 1)));
  printf("%zu\n", sizeof(&a[0] + 1));
  printf("%zu\n", sizeof(*(&a[0] + 1)));
  printf("%zu\n", sizeof(*a));
  printf("%zu\n", sizeof(a[3]));
}
//  sizeof(a), a直接交给 sizeof , 表示整个数组, 数组中共有三个类型为int[4]的元素, 所以大小是48
//  sizeof(a[0][0]) 表示数组 a 第一个数组元素的第一个元素, int 4字节
//  sizeof(a[0]) 表示数组中的首元素, 也就是第一个int[4]数组, 显然是16字节
//  sizeof(a[0] + 1)  arr[0] 表示一个int[4]数组, 那么加1就相当于指向该int[4]第一个元素的指针, int* 8字节
//  sizeof(*(a[0] + 1))  指向[0][1]的指针 int* 被再次解引用, int 4字节
//  sizeof(a + 1) 这里表示指向 a 第一个元素的指针, 丢一个元素是一个int[4], 指针为int(*)[4], 8字节
//  sizeof(*(a + 1)) 对int(*)[4]再解引用, 得到int[4], 16字节
//  sizeof(&a[0] + 1) 这是一个指针, 指向 a 第二个类型为int[4]数组的元素, 类型为int(*)[4] 8 字节
//  sizeof(*(&a[0] + 1))  对 int(*)[4] 再解引用, 得到 int[4], 16字节
//  sizeof(*a) a 是一个指针, 指向 a 第一个元素, 对其解引用会得到 int[4], 16字节
//  sizeof(a[3]), 如果存在的话, 第四个元素是 int[4]数组, 此处将数组直接当作参数, 是数组整体大小, 16

void test9(void) {
  //&a表示整个数组的地址，类型为int(*)[5]，加1跨过int[5]长度，即20字节，强制类型转换后赋给ptr，ptr类型为int*,减1越过int大小，即4字节，解引用后是数组第五个元素，即5；*(a + 1)是数组第二个元素，是2
  int a[5] = {1, 2, 3, 4, 5};
  int* ptr = (int*)(&a + 1);
  printf("%d,%d\n", *(a + 1), *(ptr - 1));
}

void test10(void)
{
	struct Test
	{
		int Num;
		char* pcName;
		short sDate;
		char cha[2];
		short sBa[4];
	}*p = (struct Test*)0x100000;//0x100000编译器认为是int，强制类型转换后变成了指针，类型为struct Text*,赋给了p
	printf("%p\n", p + 0x1);//p是指针，加1跨过指向内容的大小，即结构体struct Test的大小
	printf("%p\n", (unsigned long)p + 0x1);//p被强制转换成无符号长整型，加一就是加一
	printf("%p\n", (unsigned int*)p + 0x1);//p被强制转化成无符号整型指针，加1跨过指向内容长度，即4字节
}

void test11(void)
{
	int a[3][2] = { (0, 1), (2, 3), (4, 5) };
	int* p;
	p = a[0];
  //{}中是三个逗号表达式，逗号表达式最终结果是最后一个表达式的结果，(0,1)==1; (2,3)==3; (4,5)==5,数组内容为1,3,5,0,0,0，第一个元素是1
	printf("%d", p[0]);
}

void test12(void)
{
	int a[5][5];
	int(*p)[4];
	p = a;
	/*以a的视角来看，数组在内存中是这样的：
		[*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*]
		[^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^]
		a  a  a  a  a  a  a  a  a  a  a  a  a  a  a  a  a  a  a  a  a  a  a  a  a
		[0][0][0][0][0][1][1][1][1][1][2][2][2][2][2][3][3][3][3][3][4][4][4][4][4]
		[0][1][2][3][4][0][1][2][3][4][0][1][2][3][4][0][1][2][3][4][0][1][2][3][4]
																		 |
																		 |
															 [*][*][*][*]
															 |
		以p的视角来看，数组在内存中是这样的：                   |
		[*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*][*]
		[^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^][^]
		p  p  p  p  p  p  p  p  p  p  p  p  p  p  p  p  p  p  p  p  p  p  p  p  p
		[0][0][0][0][1][1][1][1][2][2][2][2][3][3][3][3][4][4][4][4][5][5][5][5][6]
		[0][1][2][3][0][1][2][3][0][1][2][3][0][1][2][3][0][1][2][3][0][1][2][3][0]

		指针-指针=元素个数
		&p[4][2]比 &a[4][2]小，所以&p[4][2]-&a[4][2]==-4,-4用%d的方式来表示就是-4；
		如果用%p的方式来表示，则要观察-4在计算机中的实际存储形式——补码；
		-4（x64）原码是1000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0100
				反码是1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1011
				补码是1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1100
				不过为了方便观察，地址都是以16进制来呈现的，所以还要把-4补码转化成16进制：FFFFFFFFFFFFFFFC*/

				//备注： p的定义是int(*p)[4]，指向的是int[4]的数组，a是二维数组首元素的地址，指向的应该是int[5]的数组；
				//指针-指针=元素个数，a和p指针类型不同，无法确定单位元素的大小，这个题目实际会有歧义，
				//笔试基本不会遇到这种问题，如果遇到了，对于本题这种情况就把单位元素大小当成int;如果指向的是char[4]和char[5]就把单位元素大小当成char,以此类推。
	printf("%p,%d\n", &p[4][2] - &a[4][2], &p[4][2] - &a[4][2]);
}

void test13(void)
{
	int aa[2][5] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	int* ptr1 = (int*)(&aa + 1);//&aa得到的是整个数组的地址，类型为int(*)[2][5],加一后跨过int[2][5]的大小，之后强制类型转换成int*赋给ptr1，所以ptr1的单位变化范围是int大小
	int* ptr2 = (int*)(*(aa + 1));//这里的aa是数组首元素的地址，类型为int(*)[5],加一后跨过int[5]的大小，之后强制类型转化成int*赋给ptr2,所以ptr2的单位变化范围是int大小
	printf("%d,%d", *(ptr1 - 1), *(ptr2 - 1));//综上答案为10,5
}

void test14(void)
{
	char* a[] = { "work","at","alibaba" };//a是一个指针数组，所指的指针类型都是 char*,一共三个，分别指向三个常量字符串的的首字母，char* a[] = {"work"中'w'的地址,"at"中'a'的地址,"alibaba"中的'a'的地址
	char** pa = a;//a表示数组首元素的地址，即["work"中'w'的地址]的地址
	pa++;//pa指向的是char*,加一就跳过char*的大小，由于数组中的元素地址是连续的，所以现在pa指向"at"中'a'的地址
	printf("%s\n", *pa);//综上显示at
}

void test15(void)
{
	char* c[] = { "ENTER","NEW","POINT","FIRST" };//c是一个指针数组，所指的指针类型都是 char*,一共四个，分别指向四个常量字符串的的首字母
	char** cp[] = { c + 3,c + 2,c + 1,c };
	char*** cpp = cp;//cp表示数组首元素地址，即c+3的地址
	printf("%s\n", **++cpp);
	printf("%s\n", *-- * ++cpp + 3);
	printf("%s\n", *cpp[-2] + 3);
	printf("%s\n", cpp[-1][-1] + 1);
	//指针数组c中的元素：
	//下标         内容
	// 0    "ENTER"中'E'的地址   
	// 1	"NEW"  中'N'的地址
	// 2	"POINT"中'P'的地址
	// 3    "FIRST"中'F'的地址
	//**++cpp, 前置++，先加后使用，加一后cpp指向c+2的地址，第一次解引用后变成c+2,c+2是"POINT"中'P'的地址，再解引用后变成"POINT"中'P'，打印POINT，现在cpp指向c+2的地址，cp数组内容不变：char** cp[] = { c + 3,c + 2,c + 1,c };
	//*-- * ++cpp + 3，在第一行的基础上++cpp，现在cpp指向c+1的地址，第一次解引用后变成c+1,再把c+1前置减一，变成c,再解引用后变成 "ENTER"中'E'的地址，对其+3，变成"ENTER"中第二个'E'的地址，打印ER，cp数组的内容发生改变：char** cp[] = { c + 3,c + 2,c,c };cpp指向第一个c的地址
	//*cpp[-2] + 3，cpp[-2]相当于*(cpp-2)，cpp-2指向c+3的地址，解引用后变成c+3，c+3是"FIRST"中'F'的地址,再+3就变成"FIRST"中'S'的地址，打印ST，现在cp数组内容不变：char** cp[] = { c + 3,c + 2,c,c };cpp指向第一个c的地址
	//cpp[-1][-1] + 1，cpp[-1][-1]相当于*(*(cpp-1)-1),cpp-1指向c+2的地址，解引用后变成c+2，再对其-1，变成c+1,再对其解引用变成"NEW"  中'N'的地址，再+1就变成"NEW"  中'E'的地址，打印EW，此时cp数组内容不变：char** cp[] = { c + 3,c + 2,c,c };cpp指向第一个c的地址
	//前两行中cpp指向和cp内容发生变化，是因为使用的是自加自减（++和--）
}

int main(void) {
  printSystemInfo();
  test12();
  return 0;
}
