## 七、从 main 返回，进入进程退出处理

```asm
; main 的调用者
0x7ffff7c2a1c8 <+120>: callq  *%rax
0x7ffff7c2a1ca <+122>: movl   %eax, %edi
0x7ffff7c2a1cc <+124>: callq  0x7ffff7c47ba0 ; exit

; exit
0x7ffff7c47ba0 <+0>:   endbr64
0x7ffff7c47ba4 <+4>:   pushq  %rbp
0x7ffff7c47ba5 <+5>:   movl   $0x1, %ecx
0x7ffff7c47baa <+10>:  movl   $0x1, %edx
0x7ffff7c47baf <+15>:  leaq   0x1bcaca(%rip), %rsi
0x7ffff7c47bb6 <+22>:  movq   %rsp, %rbp
0x7ffff7c47bb9 <+25>:  callq  0x7ffff7c47910 ; 对应 __run_exit_handlers
```

前面已经走过了调用 `main` 之前的准备过程。正文也已经展示了 `main` 怎样建立栈帧、调用 `Add`，以及最后怎样撤销自己的栈帧。现在，我们从 `main` 的返回位置继续向后看。

### 1. 当前由谁执行，又准备结束谁

从这里开始，执行流进入进程退出阶段。本次仍由主线程执行：它完成 `main` 的调用，接收返回值，再调用 `exit`，组织整个进程结束前的收尾。

这里需要同时留意两件事：当前执行代码的线程，以及这些操作所针对的生命周期。主线程调用 `pthread_exit`，结束的是自身线程；只要还有其他线程存活，进程就可以继续运行。而这里调用的 `exit` 将结束整个进程，进程中仍然存活的其他线程最终也会被终止。

我们正在跟踪的这条从 `main` 正常返回的路径由主线程执行。至于 `exit` 本身，任意线程都可以调用，调用者同样会触发进程退出。因此，后面这些工作的进程级作用范围，来自 `exit` 的职责。

以进程退出为目标的收尾，其中仍然包含针对当前线程的处理。接下来会看到，运行库先检查当前线程的线程局部存储（TLS）析构，再处理进程范围内登记的退出回调，随后还有动态链接器组织的模块终结以及标准 I/O 收尾。它们处在同一条退出路径中，处理对象却有不同的范围。

其他线程最终被终止，也不能据此认为它们都会完整执行各自的线程退出清理。观察当前线程实际执行的收尾时，仍然需要把这一范围保留下来。

### 2. 把 main 的返回结果交给 exit

```asm
0x7ffff7c2a1ca <+122>: movl %eax, %edi
0x7ffff7c2a1cc <+124>: callq 0x7ffff7c47ba0 ; exit
```

正文末尾，`main` 将 `%eax` 清零，撤销自己的栈帧，再执行 `retq`。先前调用 `main` 时，`callq` 压入的返回地址是 `0x7ffff7c2a1ca`，所以普通返回之后，执行流来到这里。

按照本次使用的调用约定，整数返回值放在 `%eax` 中，第一个整数参数则通过 `%edi` 传递。因此，`<+122>` 把刚刚取得的返回值复制到下一个函数接收参数的位置。随后调用 `exit`，这个数值就成为传入的退出状态。

从前后联系来看，可以写成下面这样的 C 代码：

```c
int result = main(argc, argv, __environ);
exit(result);
```

这里是对运行库调用逻辑的概括。本程序中的 `main` 返回 `0`，所以这次调用相当于 `exit(0)`。数值没有改变，它在调用关系中的作用发生了变化：先是 `main` 的返回结果，随后成为 `exit` 的实参。

至此，正文中的 `return 0` 也与进程退出联系起来。`main` 先完成自身的函数返回，运行库接过它的结果，继续承担结束进程之前的工作。

### 3. exit 准备了哪些参数

```asm
0x7ffff7c47ba0 <+0>:   endbr64
0x7ffff7c47ba4 <+4>:   pushq  %rbp
0x7ffff7c47ba5 <+5>:   movl   $0x1, %ecx
0x7ffff7c47baa <+10>:  movl   $0x1, %edx
0x7ffff7c47baf <+15>:  leaq   0x1bcaca(%rip), %rsi
0x7ffff7c47bb6 <+22>:  movq   %rsp, %rbp
0x7ffff7c47bb9 <+25>:  callq  0x7ffff7c47910
```

`exit` 的入口很短。`endbr64` 的含义前面已经讨论过，`pushq %rbp` 和 `movq %rsp, %rbp` 也仍然是在建立当前函数的栈帧。具体的退出处理被交给另一个内部函数，所以中间几条指令主要用于准备新的实参。

LLDB 在这里将目标显示为未命名的内部符号。结合 glibc 源码及后续汇编，可以把它对应到 `__run_exit_handlers`，其调用形式为：

```c
__run_exit_handlers(status, &__exit_funcs, true, true);
```

四个参数与寄存器的关系如下：

| 参数 | 传递位置 | 本次内容 | 作用 |
|---|---|---|---|
| `status` | `%edi` | `0` | 进程退出状态 |
| `listp` | `%rsi` | `&__exit_funcs` | 退出回调链表头指针变量的地址 |
| `run_list_atexit` | `%edx` | `1` | 启用退出回调处理之后的标准 I/O 收尾 |
| `run_dtors` | `%ecx`，内部检查 `%cl` | `1` | 启用当前线程的 TLS 析构处理 |

第一个参数已经放在 `%edi` 中，这几条指令没有改写它，内部函数可以继续接收同一个值。第二个参数通过 `leaq` 准备，第三、第四个参数分别写入 `%edx` 和 `%ecx`。

这也再次体现了参数寄存器的分工：上层完成一次参数传递后，当前函数又成为下一次调用的调用者，继续按约定安排寄存器。参数准备的指令顺序可以变化，只要在实际调用时，各参数已经位于规定的位置即可。

两个值为 `1` 的参数控制不同阶段。`run_dtors` 决定是否先执行当前线程的 TLS 析构处理；`run_list_atexit` 在这一实现中控制末尾是否调用 `_IO_cleanup`，进行标准 I/O 收尾。进程退出回调链表的遍历由内部函数的主体完成，不能仅凭 `run_list_atexit` 的名字，把它理解为整个回调遍历的总开关。

本次两个开关均为真。下一段进入内部函数后，就可以把这些参数与具体分支逐一对应起来。

### 4. 为什么传入链表头指针的地址

```asm
0x7ffff7c47baf <+15>: leaq 0x1bcaca(%rip), %rsi
```

这条指令使用下一条指令的地址作为 RIP 相对寻址的计算基准：

```text
0x7ffff7c47bb6 + 0x1bcaca = 0x7ffff7e04680
```

执行后，`%rsi` 中保存 `0x7ffff7e04680`。结合源码，这个位置对应 `__exit_funcs` 变量自身的地址。

先用一个简化声明表示：

```c
struct exit_function_list;

struct exit_function_list *__exit_funcs;
```

`__exit_funcs` 是一个指针变量，它保存退出回调链表的头节点地址。因此，这里存在几个需要分清的层次：

| 表达式 | 含义 |
|---|---|
| `&__exit_funcs` | 头指针变量自身的地址，本次为 `0x7ffff7e04680` |
| `__exit_funcs` | 变量中保存的头节点地址 |
| `*__exit_funcs` | 头指针非空时，它所指向的节点对象 |

本条 `leaq` 只计算第一层地址，不读取变量中保存的头节点地址。内部函数接收的形参类型也相应为：

```c
struct exit_function_list **listp;
```

为什么需要这一层间接访问？因为退出处理过程中，内部函数不仅要读取链表，还要在一个节点处理完后推进链表入口。通过传入头指针变量的地址，它就可以更新实际维护的头指针，例如：

```c
*listp = cur->next;
```

这样，调用方保存的链表入口也会随之变化。后面查看节点与内存时，这几个层次会直接体现在取地址、解引用及成员访问的指令中。

### 5. call 留下返回地址，退出流程继续向前

```asm
0x7ffff7c2a1cc <+124>: callq 0x7ffff7c47ba0 ; exit
0x7ffff7c2a1d1 <+129>: callq 0x7ffff7c99370
```

`callq exit` 仍然遵循 `call` 的机器行为，把下一条指令的地址 `0x7ffff7c2a1d1` 压栈，再转移到 `exit`。同样，`exit` 内部的调用也会把 `0x7ffff7c47bbe` 压入栈中。

不过，`exit` 在正常情况下不会返回。运行库完成收尾后，会进入最终的进程终止路径。因此，虽然栈中存在返回地址，这条正常退出路径也不会沿这些地址逐层返回到 `_start`。

后续无 Sanitizer 的对照实验中，在内部处理函数入口查看调用栈，归档记录如下：

```text
frame #0: 0x00007ffff7c47910 libc.so.6`___lldb_unnamed_symbol_47910
frame #1: 0x00007ffff7c47bbe libc.so.6`exit + 30
frame #2: 0x00007ffff7c2a1d1 libc.so.6`___lldb_unnamed_symbol_2a150 + 129
frame #3: 0x00007ffff7c2a28b libc.so.6`__libc_start_main + 139
frame #4: 0x0000555555555075 main-debug`_start + 37
```

这里保留的是该次对照实验的原始输出，`main-debug` 的代码地址属于那份二进制。我们借助这份记录观察调用关系。

其中 `exit + 30` 与调用者的 `<+129>`，对应两次 `call` 留下的返回位置。调用栈展示这些位置，并不说明它们已经执行。尤其是 `<+129>`，前面已经见过，它还有来自 `_setjmp` 非零返回分支的入口；本次 `main` 正常返回后调用 `exit`，不会再从 `exit` 普通返回到那里。

现在，退出状态、链表入口和两个控制参数都已准备完成。执行流从 `exit` 进入内部处理函数，接下来便可以看这些收尾工作怎样依次展开。

### 源码与实验依据

- 原始归档：《函数栈帧-main调用者与退出路径-原始讨论归档-2026-10-12》及《函数栈帧实验原始档案：call main 之后的正常退出路径》。汇编地址和调用栈取自归档记录。
- [glibc：libc_start_call_main.h](https://codebrowser.dev/glibc/glibc/sysdeps/nptl/libc_start_call_main.h.html)：调用 `main`、取得结果并进入 `exit` 的源码对应。
- [glibc：stdlib/exit.c](https://codebrowser.dev/glibc/glibc/stdlib/exit.c.html)：`exit` 的内部调用、四个参数以及各收尾阶段的对应关系。
