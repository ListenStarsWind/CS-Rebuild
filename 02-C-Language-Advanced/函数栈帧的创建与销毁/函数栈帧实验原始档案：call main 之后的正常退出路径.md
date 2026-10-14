# 函数栈帧实验原始档案：`call main` 之后的正常退出路径

> **档案性质**：原始实验事实、LLDB 观测、汇编对应关系与待核对事项的留存，不是正式教程，也不是最终文章正文。
>
> **研究范围**：从 glibc 调用 `main` 后接收其返回值开始，追踪 `exit` → `__run_exit_handlers` → TLS 析构检查 → 退出回调链表 → 动态链接器模块终结 → 标准 I/O 收尾 → `_exit`。不深入每个 ELF 模块自己的终结函数。
>
> **整理日期**：2026-10-14。以下原始数据来自本次会话中的 LLDB 输出；不会把推测伪装成实测。

## 0. 对象、版本与证据边界

### 0.1 实验环境与两份二进制

- Ubuntu x86-64；C17；Clang 21；CMake + Ninja；LLDB。
- **原始程序** `build/main`：2026-10-07 生成，构建包含 `-fsanitize=address,undefined`。退出回调链表中包含 Sanitizer 记录；在 LLDB 调试退出阶段时，LeakSanitizer 报告与 `ptrace` 不兼容。
- **后续对照** `build/main-debug`：2026-10-14 生成，不启用 Sanitizer；`-O0 -g3 -fno-omit-frame-pointer`，保留 DWARF 信息。原生退出流程能够正常走通。
- 另有 `build/main-debug-sanitize` 作为检测版本，与原始 `main` 不能在机器码层面直接视为同一二进制。
- 原始程序已单独保存于项目 `archive/pre-cmake-redesign/main`，并备份 `compile_commands.json` 与 SHA-256 清单。**归档中的 `CMakeLists.txt` 是从 `../demo` 复制的；由于复制时项目已替换新版配置，尚不能将它直接认定为当初构建旧 `main` 所用的原始 CMake 配置。** 是否有必要补存运行库二进制应结合归档目标再判断；仅保存主程序不能保存 glibc / 动态链接器的机器码。

**已观测的新版构建信息**：

```text
build/main-debug:          ELF 64-bit LSB pie executable, x86-64, dynamically linked,
  interpreter /lib64/ld-linux-x86-64.so.2,
  BuildID[sha1]=40f13c283da4c365d77ca7741337cec17ee3e40c,
  with debug_info, not stripped
build/main-debug-sanitize: ELF 64-bit LSB pie executable, x86-64, dynamically linked,
  interpreter /lib64/ld-linux-x86-64.so.2,
  BuildID[sha1]=8c464e7096a11e58181f33046a5ef8d2dee3e79a,
  with debug_info, not stripped
```

**定位约定**：正文优先使用同一函数内部的 `<+偏移>`；绝对虚拟地址保留在原始输出、断点命令和实测地址中。地址受 ASLR、运行库版本与构建配置影响；不可将不同实验的绝对地址拼成一次完整执行轨迹。

### 0.2 证据标识

- **[观测]**：会话中真实给出的 LLDB 输出、内存读数、进程状态。
- **[汇编推导]**：直接由已贴出的机器指令、寄存器约定及结构体偏移推得。
- **[源码对应/推定]**：依据 glibc/LLVM 实现逻辑与函数行为对应，但并非所有内部未命名符号都通过本机调试符号直接证明。
- **[未实测]**：没有提供本次运行的跳转命中、寄存器或内核系统调用直接证据，不写成“已经亲眼看见”。

---

## 1. `main` 返回后：没有沿调用栈一路 `ret` 到 `_start`

**[观测／既有反汇编]** glibc 启动代码中，调用 `main` 及其后续指令为：

```asm
<+120>: callq *%rax           ; 调用 main
<+122>: movl  %eax, %edi     ; 将 main 返回的 int 结果移入 exit 的首参数
<+124>: callq exit
<+129>: callq ___lldb_unnamed_symbol_99370
```

本程序 `main` 正常返回 `0`。按 x86-64 SysV ABI，`%eax` 中的返回值转交 `%edi`，调用 `exit(0)`。

**结论**：`exit()` 是正常情况下不返回的函数；它在用户态完成收尾，最终调用 `_exit()` 进入进程终止路径。因此 `<+129>` 虽紧邻 `call exit`，但并不是 `exit` 正常返回后会执行的“下一步”。**未单独追查 `<+129>` 的未命名目标，不能臆断它的身份。**

**[观测]** 后续 Debug 运行中，调用栈：

```text
(lldb) bt
* thread #1, name = 'main-debug', stop reason = breakpoint 4.1
  * frame #0: 0x00007ffff7c47910 libc.so.6`___lldb_unnamed_symbol_47910
    frame #1: 0x00007ffff7c47bbe libc.so.6`exit + 30
    frame #2: 0x00007ffff7c2a1d1 libc.so.6`___lldb_unnamed_symbol_2a150 + 129
    frame #3: 0x00007ffff7c2a28b libc.so.6`__libc_start_main + 139
    frame #4: 0x0000555555555075 main-debug`_start + 37
```

其中 frame #2 的 `<+129>` **是栈帧中保存的返回地址**，不能据此声称该处指令已执行。这也是“返回地址存在 ≠ callee 一定会返回”的实例。

---

## 2. `exit()` 将退出处理交给 `__run_exit_handlers`

**[源码对应/推定]** `exit(status)` 使用 `__run_exit_handlers(status, &__exit_funcs, true, true)` 进行正常退出清理。

参数语义（在当前反汇编中）：

| 入口寄存器 | 本次值/作用 | 进入后保存位置 |
|---|---|---|
| `%edi` | 退出状态码 `0` | `%r14d`，`<+12>` |
| `%rsi` | 退出回调链表头指针变量的地址 `&__exit_funcs` | `-0x38(%rbp)`，`<+24>` |
| `%edx` | `run_list_atexit = 1`；完成回调后执行标准 I/O 收尾 | `-0x4c(%rbp)`，`<+28>` |
| `%cl` | `run_dtors = 1`；决定是否先运行当前线程的 TLS 析构 | `<+31>` 检查 |

可直接观察的控制流：

```asm
<+31>:  testb %cl, %cl
<+33>:  jne   <+597>
...
<+597>: callq __call_tls_dtors
<+602>: jmp   <+39>
```

因此本次先检查 TLS 析构，再处理进程退出回调链表。

---

## 3. 当前线程 TLS 析构检查：本次列表为空

**[原先实验中的观测]** 在 `__call_tls_dtors` 中，TLS 析构链表头的加载与测试：

```asm
<+10>: movq ...(%rip), %r12
<+18>: movq %fs:(%r12), %rbx
<+23>: testq %rbx, %rbx
<+26>: je    <+94>
```

此前断点 `0x7ffff7c47707` 的寄存器观测：

```text
r12 = 0xffffffffffffff60   ; -0xa0
rbx = 0x0000000000000000
```

**[观测]** 当前线程 TLS 析构链表头为空，故此次并未进入析构记录循环。

**机制区分（之前曾混淆，现已厘清）**：

- TLS 析构链表的**每个节点对应一条析构记录**，通常先更新链表头并摘除当前节点，再调用析构函数，之后递减所属 `link_map` 的 `l_tls_dtor_count` 并释放记录节点。
- `l_tls_dtor_count` 协调的是 TLS 析构与 ELF 模块卸载之间的生命周期问题；不是当前 `__exit_funcs` 的 `idx`。
- 当前线程 TLS 对象的析构不同于整个进程退出时由动态链接器执行的 ELF 模块终结函数。

**限定**：由于列表为空，上述非空链表循环并不是本次运行的实际执行路径；它是此前阅读汇编和实现机制后的结构性说明。

---

## 4. `__exit_funcs`：进程退出回调链表

### 4.1 数据结构和精确偏移

依据本次 x86-64 glibc 布局：

```text
__exit_funcs  —— 存放链表头指针的变量
      │
      ▼
exit_function_list 节点（%r15）
  +0x000  next                   8 B   → 下一个链表节点
  +0x008  idx                    8 B   → 节点内待遍历槽位的上界
  +0x010  fns[0]                32 B
  +0x030  fns[1]                32 B
  +0x050  fns[2]                32 B
  +0x070  fns[3]                32 B
  +0x090  fns[4]                32 B
          ...
  +0x3f0  fns[31]               32 B
  +0x410  结束（总计 1040 B）
```

对 `ef_cxa` 类型，每一条 `fns[i]` 记录内部：

```text
相对于 fns[i]：
 +0x00  flavor       8 B    类型标签（long int）
 +0x08  cxa.fn       8 B    经过指针混淆的函数指针
 +0x10  cxa.arg      8 B    回调实参
 +0x18  cxa.dso_handle 8 B 模块关联标识
```

`flavor` 并非位图，而是互斥枚举型标签：`ef_free=0`、`ef_us=1`、`ef_on=2`、`ef_at=3`、`ef_cxa=4`。类型决定联合体成员解释及调用方式。

**关键区别**：节点是“批量保存最多 32 条退出回调的数组块”，不是 TLS 那种“一节点一析构记录”的链表。`idx` 表示待处理的槽位边界，**不是严格的实时有效回调数**，因为槽位可以失效。

### 4.2 旧版有 Sanitizer：`idx = 5`

**[观测]** 原始 `main`：

```text
-> <+70>: testq %r15, %r15
(lldb) register read r15
     r15 = 0x00007ffff7e05fc0
(lldb) memory read --size 8 --format d --count 1 `$r15 + 8`
0x7ffff7e05fc8: 5
```

随后读取头节点的五条记录：

```text
(lldb) memory read --size 8 --format x --count 20 `$r15 + 0x10`
0x7ffff7e05fd0: 0x0000000000000004 0x94f8bc8460ff7af6
0x7ffff7e05fe0: 0x0000000000000000 0x00005555556a5138
0x7ffff7e05ff0: 0x0000000000000004 0x94f8bc8f9e1f7af6
0x7ffff7e06000: 0x0000000000000000 0x0000000000000000
0x7ffff7e06010: 0x0000000000000004 0x94f8bc85825f7af6
0x7ffff7e06020: 0x0000000000000000 0x00005555556a5138
0x7ffff7e06030: 0x0000000000000004 0x94f8bc8f9e1f7af6
0x7ffff7e06040: 0x0000000000000000 0x0000000000000000
0x7ffff7e06050: 0x0000000000000004 0xc1adf9b7139f7af6
0x7ffff7e06060: 0x0000000000000000 0x0000000000000000
```

**[观测]** 五个 `flavor` 均为 `4`（`ef_cxa`），`arg` 均为 `NULL`。

原 LLDB Python 指针反混淆脚本通过 TLS `%fs:0x30` 的 pointer guard 还原五条指针，结果：

| 槽位 | 还原地址 | LLDB 查出的目标 |
|---|---|---|
| `fns[0]` | `0x555555651a30` | `__sanitizer_cov_dump`（main 映像） |
| `fns[1]` | `0x55555560e540` | `AtCxaAtexit(void*)`（main 映像） |
| `fns[2]` | `0x55555565eb60` | `__lsan::DoLeakCheck()`（main 映像） |
| `fns[3]` | `0x55555560e540` | `AtCxaAtexit(void*)`（main 映像） |
| `fns[4]` | `0x7ffff7fca380` | `___lldb_unnamed_symbol_5380`（ld-linux 映像） |

符号虽显示于 `main` 映像中，但前四条属于链接进程序的 LLVM Sanitizer 运行时功能；不表示源代码自己定义了这些函数。`fns[1]` 与 `fns[3]` 的目标相同。

### 4.3 新版普通 Debug：`idx = 1`

**[观测]** 在无 Sanitizer 的 `main-debug` 中再次进入相同退出处理函数：

```text
-> <+70>: testq %r15, %r15
(lldb) register read r15
     r15 = 0x00007ffff7e05fc0
(lldb) memory read --size 8 --format d --count 1 `$r15 + 8`
0x7ffff7e05fc8: 1

(lldb) memory read --size 8 --format x --count 4 `$r15 + 0x10`
0x7ffff7e05fd0: 0x0000000000000004 0x268b2222b814d560
0x7ffff7e05fe0: 0x0000000000000000 0x0000000000000000
```

**[观测]** 唯一已占用槽位是 `fns[0]`，类型为 `ef_cxa`；与旧版相同的是没有非空参数。这里尚未证明是否还有后继节点，但当前实际运行在处理回调后已到达 `<+256>` 的完成路径。

**[观测]** 在回调调用之前停于 `<+356>`：

```text
-> 0x7ffff7c47a74 <+356>: callq *%rax
(lldb) register read rax
     rax = 0x00007ffff7fca380  ld-linux-x86-64.so.2`___lldb_unnamed_symbol_5380
(lldb) image lookup --address 0x00007ffff7fca380
      Address: ld-linux-x86-64.so.2[0x0000000000005380]
      Summary: ld-linux-x86-64.so.2`___lldb_unnamed_symbol_5380
```

**[源码对应/推定]** 此动态链接器内偏移 `0x5380` 的函数为 `_dl_fini`。LLDB 未提供该函数的真实符号名，但此前其内部 `_rtld_global` 使用、模块列表遍历、排序与终结调用路径都与 `_dl_fini` 相符。不能把工具输出说成“直接打印了 `_dl_fini`”。

---

## 5. 一个退出回调如何被取出、调用和完成

### 5.1 获取锁、选择数组槽位、分派类型

**[汇编]** `__run_exit_handlers`：

```asm
<+39>:  leaq ...(%rip), %rbx       ; __exit_funcs_lock 地址
<+46>:  xorl %eax, %eax
<+48>:  movl $1, %edx
<+53>:  lock
<+54>:  cmpxchgl %edx, (%rbx)   ; 尝试获取锁
<+57>:  jne <+630>              ; 有竞争则等待
<+63>:  movq -0x38(%rbp), %rax  ; &__exit_funcs
<+67>:  movq (%rax), %r15       ; 当前头节点
<+70>:  testq %r15, %r15
<+73>:  je <+256>
<+79>:  xorl %r13d, %r13d
<+88>:  movq 0x8(%r15), %rax    ; idx
<+92>:  testq %rax, %rax
<+95>:  je <+456>              ; idx 为 0：节点处理完成
<+101>: subq $1, %rax         ; 先 idx - 1
<+105>: movq ...(%rip), %r12  ; 记录 __new_exitfn_called 旧值
<+112>: movq %rax, 0x8(%r15)  ; 更新 idx
<+116>: shlq $5, %rax         ; 索引 × 32
<+120>: addq %r15, %rax       ; 节点地址 + 偏移
<+123>: movq 0x10(%rax), %rcx ; 取 fns[idx].flavor
<+127>: cmpq $3, %rcx
<+131>: je <+392>              ; ef_at
<+137>: cmpq $4, %rcx
<+141>: je <+304>              ; ef_cxa，本次实际走此分支
<+147>: cmpq $2, %rcx
<+151>: jne <+88>              ; 非 ef_on：跳过
```

说明：`%rax` 在 `<+120>` 后是 `node + idx*32`，并非最终 `fns[idx]` 本身的起始地址；随后指令中的额外 `+0x10` 才补上数组起始偏移。

### 5.2 `ef_cxa` 实际分支：还原函数指针，先失效再调用

```asm
<+304>: movq 0x18(%rax), %rcx   ; 取得 cxa.fn 的混淆值
<+308>: movq 0x20(%rax), %r8    ; cxa.arg
<+312>: movq $0, 0x10(%rax)    ; flavor = ef_free
<+320>: movq %rcx, %rax
<+323>: movl %r13d, %ecx       ; 0，准备解锁
<+326>: rorq $0x11, %rax
<+330>: xorq %fs:0x30, %rax    ; pointer guard 反混淆
<+339>: xchgl %ecx, (%rbx)     ; 释放 __exit_funcs_lock
<+341>: cmpl $1, %ecx
<+344>: jg <+568>               ; 有锁竞争时唤醒等待者
<+350>: movl %r14d, %esi       ; status，当前为 0
<+353>: movq %r8, %rdi         ; arg，当前为 NULL
<+356>: callq *%rax            ; 实际回调：动态链接器内 +0x5380
<+358>: movl $1, %ecx          ; 回调返回后，准备重新加锁
<+363>: movl %r13d, %eax       ; 0
<+366>: lock
<+367>: cmpxchgl %ecx, (%rbx)
<+370>: je <+214>
<+376>: movq %rbx, %rdi
<+379>: callq __lll_lock_wait_private
<+384>: jmp <+214>
```

**[汇编推导]** 提前递减 `idx`，并把 `flavor` 改为 `ef_free`，防止同一记录在递归或复杂注册路径中被误认为尚待处理。执行回调前释放链表锁，允许回调间接触发新的注册；返回后重新获取锁。

**[汇编]** 比较登记计数快照：

```asm
<+214>: movq ...(%rip), %rax  ; 当前 __new_exitfn_called
<+221>: cmpq %r12, %rax       ; 与回调前快照比较
<+224>: je <+88>              ; 未变化：继续原节点
<+230>: movq -0x38(%rbp), %rax
<+234>: movq (%rax), %r15     ; 有新增时重新取头节点
<+237>: testq %r15, %r15
<+240>: jne <+88>
```

这**不是泛称“防重入”**；计数器的针对性功能是检测回调执行期间是否新增登记，以免错过新回调。互斥锁处理并发访问，二者不是一回事。

### 5.3 锁竞争分支与 futex（只留机制，不深究）

```asm
<+568>: movq %rbx, %rdi
<+571>: movq %rax, -0x48(%rbp)
<+575>: movq %r8, -0x40(%rbp)
<+579>: callq __lll_lock_wake_private
<+584>: movq -0x48(%rbp), %rax
<+588>: movq -0x40(%rbp), %r8
<+592>: jmp <+350>
```

此分支在解锁时发现竞争才需要执行。`__lll_lock_wake_private` 底层使用 `futex` 唤醒最多一个等待者；唤醒并不等于立即获得锁，等待者还需重新参与调度和争用。**本次对 `<+617>` 设置断点没有命中**，与单线程无锁竞争的情况相符。不要把未命中的竞争分支写成已执行路径。

---

## 6. 动态链接器回调：`_dl_fini`（宏观了解，不追各 ELF 模块内部）

### 6.1 身份证据

**[观测]** 旧 `main` 的 `fns[4]` 和新 `main-debug` 的 `fns[0]`，反混淆后均指向：

```text
0x00007ffff7fca380
ld-linux-x86-64.so.2 + 0x5380
LLDB: ___lldb_unnamed_symbol_5380
```

**[原始反汇编入口]**：

```asm
<+0>:  endbr64
<+4>:  pushq %rbp
<+5>:  movq %rsp, %rbp
<+8>:  pushq %r15
<+10>: pushq %r14
<+12>: xorl %r14d, %r14d
<+15>: pushq %r13
<+17>: leaq ...(%rip), %r13 ; _rtld_global + 2568
<+24>: pushq %r12
<+26>: pushq %rbx
<+27>: subq $0x28, %rsp
<+31>: movl $0x2, -0x4c(%rbp)
<+38>: movq ...(%rip), %rax ; _rtld_global + 2560
<+45>: movq %rax, %r12
<+48>: subq $0x1, %r12
<+52>: js <+593>
```

**[源码对应/推定]** 这正是 `_dl_fini` 一类动态链接器终结入口的结构：遍历动态链接命名空间、收集待终结 ELF 对象、基于依赖关系排序、调用其终结函数、维护加载器状态。

### 6.2 作用边界

- `_dl_fini` 是**进程正常终止阶段的 ELF 模块终结调度器**，不是一个工作线程结束时的 TLS 清理。
- 它主要组织并调用各对象的 `.fini_array` / `DT_FINI` 终结函数。具体清理何种资源由各模块自己的终结代码决定。
- **不等于立刻卸载全部共享库，也不负责直接销毁进程地址空间**；从调用返回到最终 `_exit`，进程仍有有效用户态代码与数据可继续执行。
- 模块依赖关系可能影响终结顺序。用户决定只作宏观确认，不深入模块排序算法、审计回调、每个 ELF 模块的终结函数内部。

### 6.3 已贴出的 `_dl_fini` 汇编分区索引（非逐行解释）

| 函数内偏移 | 大致职责（源码对应） |
|---|---|
| `<+0>`～`<+81>` | 初始化状态、选取动态链接命名空间 |
| `<+88>`～`<+150>` | 保护/检查命名空间的模块状态 |
| `<+152>`～`<+374>` | 建立本轮模块指针数组及排序准备 |
| `<+375>`～`<+406>` | 排列模块终结顺序，随后退出受锁保护区 |
| `<+407>`～`<+551>` | 遍历对象，执行模块终结及可选审计通知 |
| `<+560>`～`<+656>` | 最后的可选处理、函数返回 |
| `<+658>`～`<+746>` | 一致性检查失败的异常路径 |

此前推定 `<+389>` 对应排序函数，`<+499>` 对应 `_dl_call_fini`，`<+507>` 对应审计通知；**具体内部函数名没有通过当前 LLDB 的符号解析直接确认**。

---

## 7. `_dl_fini` 返回、重新加锁与耗尽链表

**[观测]** 在新版 `main-debug` 上，断点确实命中了回调调用之后的 `<+358>`：

```text
-> 0x7ffff7c47a76 <+358>: movl $0x1, %ecx
   0x7ffff7c47a7b <+363>: movl %r13d, %eax
   0x7ffff7c47a7e <+366>: lock
   0x7ffff7c47a7f <+367>: cmpxchgl %ecx, (%rbx)
```

因此在这一版中，**动态链接器终结回调确实正常返回到了 `__run_exit_handlers`**。随后重新加锁，按 `<+214>` 的计数器比较继续检查回调链表。

**[观测]** 后续实际停在 `<+456>`：

```asm
-> <+456>: movq (%r15), %rax
   <+459>: movq -0x38(%rbp), %rdx
   <+463>: movq %rax, (%rdx)
   <+466>: testq %rax, %rax
   <+469>: je <+256>
   <+475>: movq %r15, %rdi
   <+478>: callq ___lldb_unnamed_symbol_283e0
```

**[汇编推导]** `idx == 0` 时，不再从节点内取出任何回调，而是：

1. `next = cur->next`；
2. `*listp = next`：从链表头摘除旧节点；
3. 如果 `next == NULL`，直接到 `<+256>`；
4. 如果 `next != NULL`，调用与 `free` 对应的释放入口 `<+478>`，重新获取头指针并继续遍历。

**注意**：末端初始节点是**静态分配且本身容纳回调记录**的节点，不是纯粹只占位的哨兵。链表尾节点不调用 `free`。这里 `<+478>` 与之前 TLS 链表中的释放调用是同一 glibc 地址。

---

## 8. 退出回调处理完成：状态标志、解锁、stdio 收尾

**[观测]** 实际停在 `<+256>`：

```asm
-> <+256>: xorl  %eax, %eax
   <+258>: movb  $0x1, 0x1be9cf(%rip)
   <+265>: xchgl %eax, (%rbx)
   <+267>: cmpl  $0x1, %eax
   <+270>: jg    <+617>
   <+276>: cmpb  $0x0, -0x4c(%rbp)
   <+280>: jne   <+607>
   <+286>: movl  %r14d, %edi
   <+289>: callq _exit
```

对应含义：

- `<+258>`：设置退出回调管理完成标志（按 glibc 实现对应 `__exit_funcs_done`）。
- `<+265>`～`<+270>`：释放 `__exit_funcs_lock`；若有等待者，则进入 `<+617>` 的唤醒分支。
- **[观测]** 曾对 `<+617>` 设置断点后 `continue`，程序直接 `exited with status = 0`，**未命中**该竞争唤醒路径。
- `<+276>`～`<+280>`：检查进入函数时保存的 `run_list_atexit`，本次为 `1`，因此进入 `<+607>`。

**[观测]** 后续确实命中：

```asm
-> <+607>: callq 0x7ffff7c968c0 ; ___lldb_unnamed_symbol_968c0
   <+612>: jmp   <+286>
```

**[源码对应/推定]** 该未命名函数是 glibc 的 `_IO_cleanup`：对 glibc stdio 的 `FILE` 流执行整体收尾，核心包括刷新缓冲输出及缓冲管理收尾。**不是逐个关闭进程的所有文件描述符**；不要将它与内核最后回收文件资源混为一谈。

用户决定**不再进入 `_IO_cleanup` 逐条阅读反汇编**；事后总结认为，对这种库函数更有效的研究顺序应当是先读对应实现源码，再以少量关键汇编和 LLDB 读数验证。

---

## 9. `_exit`：不再返回，进程在内核中终止

**[既有反汇编]**：

```asm
<+286>: movl %r14d, %edi
<+289>: callq _exit
```

**[源码/平台机制]** 在此 Linux/glibc 环境下，`_exit` 最终请求 `exit_group` 系统调用，使内核终止整个线程组（进程）。这不同于仅终止一个线程的内核 `exit` 系统调用，也不同于 glibc `exit()` 的用户态回调与 stdio 清理。

**[观测]** 不启用 Sanitizer 的 `main-debug` 多次正常 `exited with status = 0`。未提供对 `_exit` 内部 `syscall` 指令的 LLDB 单步输出；`exit_group` 为运行时/内核实现解释，**不标注为本次直接观察到的系统调用寄存器证据**。

整体控制流闭合：

```text
main() 返回 0
    ↓
glibc 启动代码：mov %eax,%edi → exit(0)
    ↓
__run_exit_handlers(...)
    ├─ __call_tls_dtors             [旧实验实测 TLS 记录为空]
    ├─ __exit_funcs 遍历
    │    └─ 动态链接器退出回调      [旧版 fns[4]；新版 fns[0]]
    │         └─ _dl_fini（身份为源码/行为对应）
    │              └─ ELF 模块终结（按需）
    ├─ 完成标志、解锁
    └─ _IO_cleanup                  [命中 <+607>]
    ↓
_exit(status=0)
    ↓
Linux exit_group（实现机制）
    ↓
内核终止进程；不再沿栈返回至 _start
```

---

## 10. 旧版 Sanitizer 的干扰事件：应作为实验异常保留

**[观测]** 旧 `main` 停在 `__run_exit_handlers <+356>`，无论 `thread step-inst-over` 或继续执行，都曾出现：

```text
==2568828==LeakSanitizer has encountered a fatal error.
==2568828==HINT: For debugging, try setting environment variable LSAN_OPTIONS=verbosity=1:log_threads=1
==2568828==HINT: LeakSanitizer does not work under ptrace (strace, gdb, etc)
Process 2568828 exited with status = -1 (0xffffffff) lost connection
```

后续重新启动，也观察到：

```text
-> 0x7ffff7c47a74 <+356>: callq *%rax
(lldb) continue
==2583256==LeakSanitizer has encountered a fatal error.
==2583256==HINT: LeakSanitizer does not work under ptrace (strace, gdb, etc)
Process 2583256 exited with status = 1 (0x00000001)
```

即使在 `<+358>` 设置断点，也没有命中：

```text
6: address = libc.so.6[0x0000000000047a76]
   ... + 358, hit count = 0
```

**严谨的判断**：LSan 与 `ptrace` 的冲突确实存在，导致旧版调试退出流程不可靠；但不能凭此推断 `_dl_fini` 自身抛出了错误，亦不能把 LSan 错误解释成 glibc 的正常退出行为。

**工程处置**：保留旧 `main` 作为历史证据；双版本构建 `main-debug`（无 Sanitizer）/`main-debug-sanitize`（动态检测）分离。新版 `main-debug` 已实测能正常返回 `<+358>` 并以状态码 `0` 终止。

---

## 11. 研究结论与边界（供日后写作提炼）

1. `main` 返回不是用户态调用栈逐层正常 `ret` 到 `_start`；glibc 启动代码调用 `exit(status)` 并最终走到不返回的 `_exit`。
2. 线程 TLS 析构与进程级 ELF 模块终结**不是同一层生命周期**。前者操作当前线程的析构记录，后者由动态链接器在正常进程退出阶段调度模块终结函数。
3. `__exit_funcs` 是“链表节点 + 每节点固定 32 项回调数组”，`idx` 控制节点内部待处理槽位上界，不能简单视为有效回调数量。
4. 执行回调时先更新槽位状态，释放管理锁，执行后重新加锁；`__new_exitfn_called` 的前后比较是为处理回调执行期间**新增登记**，不是笼统的“防重入”。
5. 在原始 Sanitizer 程序中，退出记录共五条；在普通 Debug 程序中，当前节点仅一条，指向相同动态链接器内部入口。
6. `_dl_fini` 的职责是执行各 ELF 模块的**终结函数**，不等于立即卸载所有模块，更不等于直接回收整个进程地址空间。
7. `_IO_cleanup` 与 `_exit` 属于不同层次：前者 stdio 用户态清理，后者通过系统调用交由内核终止进程。
8. **实验方法调整**：以后调查大型运行库内部函数，优先读与本机版本相符的源码；反汇编仅核对关键分支、ABI 参数与实测控制流。避免机械地逐行死磕。

### 尚未完成或不宜做出的断言

- 本次未记录 `getconf GNU_LIBC_VERSION` 的输出，不应未经验证便写死 glibc 精确补丁版本。
- `ld-linux...+0x5380`、`libc...+0x968c0`、`libc...+0x283e0` 的语义识别来自源码与行为对应，LLDB 中保留的是 unnamed_symbol，不能冒充直接解析出的名称。
- 未提供这次主程序所有 `.fini_array` 内容及每个库的实际终结函数调用清单；不应列出臆测的已执行模块清理操作。
- 未直接单步观察到 `exit_group` 的 `syscall` 指令；该部分属于 Linux/glibc 实现机制解释。
- 绝对运行时地址不保证能跨进程重启直接复用；原始实验的指令偏移及原始二进制应保持一致地引用。

---

## 附录 A：`__run_exit_handlers` 全量原始反汇编（会话留存）

> 本段按用户在本次会话粘贴的 LLDB 输出转录，保留全部指令、偏移和地址；仅规范缩进并省略停靠箭头，便于以后核对文章中的指令。不代表所有分支在本次运行中都执行过。

```asm
libc.so.6`___lldb_unnamed_symbol_47910:
  0x7ffff7c47910 <+0>:   endbr64
  0x7ffff7c47914 <+4>:   pushq  %rbp
  0x7ffff7c47915 <+5>:   movq   %rsp, %rbp
  0x7ffff7c47918 <+8>:   pushq  %r15
  0x7ffff7c4791a <+10>:  pushq  %r14
  0x7ffff7c4791c <+12>:  movl   %edi, %r14d
  0x7ffff7c4791f <+15>:  pushq  %r13
  0x7ffff7c47921 <+17>:  pushq  %r12
  0x7ffff7c47923 <+19>:  pushq  %rbx
  0x7ffff7c47924 <+20>:  subq   $0x28, %rsp
  0x7ffff7c47928 <+24>:  movq   %rsi, -0x38(%rbp)
  0x7ffff7c4792c <+28>:  movl   %edx, -0x4c(%rbp)
  0x7ffff7c4792f <+31>:  testb  %cl, %cl
  0x7ffff7c47931 <+33>:  jne    0x7ffff7c47b65 ; <+597>
  0x7ffff7c47937 <+39>:  leaq   0x1be66a(%rip), %rbx
  0x7ffff7c4793e <+46>:  xorl   %eax, %eax
  0x7ffff7c47940 <+48>:  movl   $0x1, %edx
  0x7ffff7c47945 <+53>:  lock
  0x7ffff7c47946 <+54>:  cmpxchgl %edx, (%rbx)
  0x7ffff7c47949 <+57>:  jne    0x7ffff7c47b86 ; <+630>
  0x7ffff7c4794f <+63>:  movq   -0x38(%rbp), %rax
  0x7ffff7c47953 <+67>:  movq   (%rax), %r15
  0x7ffff7c47956 <+70>:  testq  %r15, %r15
  0x7ffff7c47959 <+73>:  je     0x7ffff7c47a10 ; <+256>
  0x7ffff7c4795f <+79>:  xorl   %r13d, %r13d
  0x7ffff7c47962 <+82>:  nopw   (%rax,%rax)
  0x7ffff7c47968 <+88>:  movq   0x8(%r15), %rax
  0x7ffff7c4796c <+92>:  testq  %rax, %rax
  0x7ffff7c4796f <+95>:  je     0x7ffff7c47ad8 ; <+456>
  0x7ffff7c47975 <+101>: subq   $0x1, %rax
  0x7ffff7c47979 <+105>: movq   0x1be620(%rip), %r12
  0x7ffff7c47980 <+112>: movq   %rax, 0x8(%r15)
  0x7ffff7c47984 <+116>: shlq   $0x5, %rax
  0x7ffff7c47988 <+120>: addq   %r15, %rax
  0x7ffff7c4798b <+123>: movq   0x10(%rax), %rcx
  0x7ffff7c4798f <+127>: cmpq   $0x3, %rcx
  0x7ffff7c47993 <+131>: je     0x7ffff7c47a98 ; <+392>
  0x7ffff7c47999 <+137>: cmpq   $0x4, %rcx
  0x7ffff7c4799d <+141>: je     0x7ffff7c47a40 ; <+304>
  0x7ffff7c479a3 <+147>: cmpq   $0x2, %rcx
  0x7ffff7c479a7 <+151>: jne    0x7ffff7c47968 ; <+88>
  0x7ffff7c479a9 <+153>: movq   0x18(%rax), %rcx
  0x7ffff7c479ad <+157>: movq   0x20(%rax), %rsi
  0x7ffff7c479b1 <+161>: movq   %rcx, %rax
  0x7ffff7c479b4 <+164>: movl   %r13d, %ecx
  0x7ffff7c479b7 <+167>: rorq   $0x11, %rax
  0x7ffff7c479bb <+171>: xorq   %fs:0x30, %rax
  0x7ffff7c479c4 <+180>: xchgl  %ecx, (%rbx)
  0x7ffff7c479c6 <+182>: cmpl   $0x1, %ecx
  0x7ffff7c479c9 <+185>: jg     0x7ffff7c47b28 ; <+536>
  0x7ffff7c479cf <+191>: movl   %r14d, %edi
  0x7ffff7c479d2 <+194>: callq  *%rax
  0x7ffff7c479d4 <+196>: movl   $0x1, %ecx
  0x7ffff7c479d9 <+201>: movl   %r13d, %eax
  0x7ffff7c479dc <+204>: lock
  0x7ffff7c479dd <+205>: cmpxchgl %ecx, (%rbx)
  0x7ffff7c479e0 <+208>: jne    0x7ffff7c47a88 ; <+376>
  0x7ffff7c479e6 <+214>: movq   0x1be5b3(%rip), %rax
  0x7ffff7c479ed <+221>: cmpq   %r12, %rax
  0x7ffff7c479f0 <+224>: je     0x7ffff7c47968 ; <+88>
  0x7ffff7c479f6 <+230>: movq   -0x38(%rbp), %rax
  0x7ffff7c479fa <+234>: movq   (%rax), %r15
  0x7ffff7c479fd <+237>: testq  %r15, %r15
  0x7ffff7c47a00 <+240>: jne    0x7ffff7c47968 ; <+88>
  0x7ffff7c47a06 <+246>: nopw   %cs:(%rax,%rax)
  0x7ffff7c47a10 <+256>: xorl   %eax, %eax
  0x7ffff7c47a12 <+258>: movb   $0x1, 0x1be9cf(%rip)
  0x7ffff7c47a19 <+265>: xchgl  %eax, (%rbx)
  0x7ffff7c47a1b <+267>: cmpl   $0x1, %eax
  0x7ffff7c47a1e <+270>: jg     0x7ffff7c47b79 ; <+617>
  0x7ffff7c47a24 <+276>: cmpb   $0x0, -0x4c(%rbp)
  0x7ffff7c47a28 <+280>: jne    0x7ffff7c47b6f ; <+607>
  0x7ffff7c47a2e <+286>: movl   %r14d, %edi
  0x7ffff7c47a31 <+289>: callq  0x7ffff7cee300 ; _exit
  0x7ffff7c47a36 <+294>: nopw   %cs:(%rax,%rax)
  0x7ffff7c47a40 <+304>: movq   0x18(%rax), %rcx
  0x7ffff7c47a44 <+308>: movq   0x20(%rax), %r8
  0x7ffff7c47a48 <+312>: movq   $0x0, 0x10(%rax)
  0x7ffff7c47a50 <+320>: movq   %rcx, %rax
  0x7ffff7c47a53 <+323>: movl   %r13d, %ecx
  0x7ffff7c47a56 <+326>: rorq   $0x11, %rax
  0x7ffff7c47a5a <+330>: xorq   %fs:0x30, %rax
  0x7ffff7c47a63 <+339>: xchgl  %ecx, (%rbx)
  0x7ffff7c47a65 <+341>: cmpl   $0x1, %ecx
  0x7ffff7c47a68 <+344>: jg     0x7ffff7c47b48 ; <+568>
  0x7ffff7c47a6e <+350>: movl   %r14d, %esi
  0x7ffff7c47a71 <+353>: movq   %r8, %rdi
  0x7ffff7c47a74 <+356>: callq  *%rax
  0x7ffff7c47a76 <+358>: movl   $0x1, %ecx
  0x7ffff7c47a7b <+363>: movl   %r13d, %eax
  0x7ffff7c47a7e <+366>: lock
  0x7ffff7c47a7f <+367>: cmpxchgl %ecx, (%rbx)
  0x7ffff7c47a82 <+370>: je     0x7ffff7c479e6 ; <+214>
  0x7ffff7c47a88 <+376>: movq   %rbx, %rdi
  0x7ffff7c47a8b <+379>: callq  0x7ffff7c98fc0 ; __lll_lock_wait_private
  0x7ffff7c47a90 <+384>: jmp    0x7ffff7c479e6 ; <+214>
  0x7ffff7c47a95 <+389>: nopl   (%rax)
  0x7ffff7c47a98 <+392>: movl   %r13d, %ecx
  0x7ffff7c47a9b <+395>: movq   0x18(%rax), %rax
  0x7ffff7c47a9f <+399>: rorq   $0x11, %rax
  0x7ffff7c47aa3 <+403>: xorq   %fs:0x30, %rax
  0x7ffff7c47aac <+412>: xchgl  %ecx, (%rbx)
  0x7ffff7c47aae <+414>: cmpl   $0x1, %ecx
  0x7ffff7c47ab1 <+417>: jg     0x7ffff7c47b10 ; <+512>
  0x7ffff7c47ab3 <+419>: callq  *%rax
  0x7ffff7c47ab5 <+421>: movl   $0x1, %ecx
  0x7ffff7c47aba <+426>: movl   %r13d, %eax
  0x7ffff7c47abd <+429>: lock
  0x7ffff7c47abe <+430>: cmpxchgl %ecx, (%rbx)
  0x7ffff7c47ac1 <+433>: je     0x7ffff7c479e6 ; <+214>
  0x7ffff7c47ac7 <+439>: movq   %rbx, %rdi
  0x7ffff7c47aca <+442>: callq  0x7ffff7c98fc0 ; __lll_lock_wait_private
  0x7ffff7c47acf <+447>: jmp    0x7ffff7c479e6 ; <+214>
  0x7ffff7c47ad4 <+452>: nopl   (%rax)
  0x7ffff7c47ad8 <+456>: movq   (%r15), %rax
  0x7ffff7c47adb <+459>: movq   -0x38(%rbp), %rdx
  0x7ffff7c47adf <+463>: movq   %rax, (%rdx)
  0x7ffff7c47ae2 <+466>: testq  %rax, %rax
  0x7ffff7c47ae5 <+469>: je     0x7ffff7c47a10 ; <+256>
  0x7ffff7c47aeb <+475>: movq   %r15, %rdi
  0x7ffff7c47aee <+478>: callq  0x7ffff7c283e0 ; ___lldb_unnamed_symbol_283e0
  0x7ffff7c47af3 <+483>: movq   -0x38(%rbp), %rax
  0x7ffff7c47af7 <+487>: movq   (%rax), %r15
  0x7ffff7c47afa <+490>: testq  %r15, %r15
  0x7ffff7c47afd <+493>: jne    0x7ffff7c47968 ; <+88>
  0x7ffff7c47b03 <+499>: jmp    0x7ffff7c47a10 ; <+256>
  0x7ffff7c47b08 <+504>: nopl   (%rax,%rax)
  0x7ffff7c47b10 <+512>: movq   %rbx, %rdi
  0x7ffff7c47b13 <+515>: movq   %rax, -0x40(%rbp)
  0x7ffff7c47b17 <+519>: callq  0x7ffff7c99080 ; __lll_lock_wake_private
  0x7ffff7c47b1c <+524>: movq   -0x40(%rbp), %rax
  0x7ffff7c47b20 <+528>: jmp    0x7ffff7c47ab3 ; <+419>
  0x7ffff7c47b22 <+530>: nopw   (%rax)
  0x7ffff7c47b28 <+536>: movq   %rbx, %rdi
  0x7ffff7c47b2b <+539>: movq   %rax, -0x48(%rbp)
  0x7ffff7c47b2f <+543>: movq   %rsi, -0x40(%rbp)
  0x7ffff7c47b33 <+547>: callq  0x7ffff7c99080 ; __lll_lock_wake_private
  0x7ffff7c47b38 <+552>: movq   -0x48(%rbp), %rax
  0x7ffff7c47b3c <+556>: movq   -0x40(%rbp), %rsi
  0x7ffff7c47b40 <+560>: jmp    0x7ffff7c479cf ; <+191>
  0x7ffff7c47b45 <+565>: nopl   (%rax)
  0x7ffff7c47b48 <+568>: movq   %rbx, %rdi
  0x7ffff7c47b4b <+571>: movq   %rax, -0x48(%rbp)
  0x7ffff7c47b4f <+575>: movq   %r8, -0x40(%rbp)
  0x7ffff7c47b53 <+579>: callq  0x7ffff7c99080 ; __lll_lock_wake_private
  0x7ffff7c47b58 <+584>: movq   -0x48(%rbp), %rax
  0x7ffff7c47b5c <+588>: movq   -0x40(%rbp), %r8
  0x7ffff7c47b60 <+592>: jmp    0x7ffff7c47a6e ; <+350>
  0x7ffff7c47b65 <+597>: callq  0x7ffff7c476f0 ; __call_tls_dtors
  0x7ffff7c47b6a <+602>: jmp    0x7ffff7c47937 ; <+39>
  0x7ffff7c47b6f <+607>: callq  0x7ffff7c968c0 ; ___lldb_unnamed_symbol_968c0
  0x7ffff7c47b74 <+612>: jmp    0x7ffff7c47a2e ; <+286>
  0x7ffff7c47b79 <+617>: movq   %rbx, %rdi
  0x7ffff7c47b7c <+620>: callq  0x7ffff7c99080 ; __lll_lock_wake_private
  0x7ffff7c47b81 <+625>: jmp    0x7ffff7c47a24 ; <+276>
  0x7ffff7c47b86 <+630>: movq   %rbx, %rdi
  0x7ffff7c47b89 <+633>: callq  0x7ffff7c98fc0 ; __lll_lock_wait_private
  0x7ffff7c47b8e <+638>: jmp    0x7ffff7c4794f ; <+63>
  0x7ffff7c47b93 <+643>: nopw   %cs:(%rax,%rax)
  0x7ffff7c47b9e <+654>: nop
```

## 附录 B：动态链接器终结回调的完整原始反汇编（仅存档，不作逐行讲解）

> 以下为本次会话中用户提供的 `ld-linux-x86-64.so.2` 内部偏移 `0x5380` 对应完整 `disassemble --frame`，不等于所有分支都被实际走过。

```asm
ld-linux-x86-64.so.2`___lldb_unnamed_symbol_5380:
  0x7ffff7fca380 <+0>:   endbr64
  0x7ffff7fca384 <+4>:   pushq  %rbp
  0x7ffff7fca385 <+5>:   movq   %rsp, %rbp
  0x7ffff7fca388 <+8>:   pushq  %r15
  0x7ffff7fca38a <+10>:  pushq  %r14
  0x7ffff7fca38c <+12>: xorl   %r14d, %r14d
  0x7ffff7fca38f <+15>: pushq  %r13
  0x7ffff7fca391 <+17>: leaq   0x33670(%rip), %r13 ; _rtld_global + 2568
  0x7ffff7fca398 <+24>: pushq  %r12
  0x7ffff7fca39a <+26>: pushq  %rbx
  0x7ffff7fca39b <+27>: subq   $0x28, %rsp
  0x7ffff7fca39f <+31>: movl   $0x2, -0x4c(%rbp)
  0x7ffff7fca3a6 <+38>: movq   0x33653(%rip), %rax ; _rtld_global + 2560
  0x7ffff7fca3ad <+45>: movq   %rax, %r12
  0x7ffff7fca3b0 <+48>: subq   $0x1, %r12
  0x7ffff7fca3b4 <+52>: js     0x7ffff7fca5d1 ; <+593>
  0x7ffff7fca3ba <+58>: leaq   (%rax,%rax,4), %rax
  0x7ffff7fca3be <+62>: leaq   0x32c3b(%rip), %rsi ; _rtld_global
  0x7ffff7fca3c5 <+69>: shlq   $0x5, %rax
  0x7ffff7fca3c9 <+73>: leaq   -0xa0(%rsi,%rax), %rbx
  0x7ffff7fca3d1 <+81>: jmp    0x7ffff7fca3f6 ; <+118>
  0x7ffff7fca3d3 <+83>: nopl   (%rax,%rax)
  0x7ffff7fca3d8 <+88>: movq   %r13, %rdi
  0x7ffff7fca3db <+91>: callq  *0x32647(%rip)
  0x7ffff7fca3e1 <+97>: subq   $0x1, %r12
  0x7ffff7fca3e5 <+101>: subq   $0xa0, %rbx
  0x7ffff7fca3ec <+108>: cmpq   $-0x1, %r12
  0x7ffff7fca3f0 <+112>: je     0x7ffff7fca5b0 ; <+560>
  0x7ffff7fca3f6 <+118>: movq   %r13, %rdi
  0x7ffff7fca3f9 <+121>: callq  *0x32631(%rip)
  0x7ffff7fca3ff <+127>: movl   0x8(%rbx), %ecx
  0x7ffff7fca402 <+130>: testl  %ecx, %ecx
  0x7ffff7fca404 <+132>: je     0x7ffff7fca3d8 ; <+88>
  0x7ffff7fca406 <+134>: movq   (%rbx), %rax
  0x7ffff7fca409 <+137>: movzbl 0x356(%rax), %eax
  0x7ffff7fca410 <+144>: andl   $0x1, %eax
  0x7ffff7fca413 <+147>: cmpl   %r14d, %eax
  0x7ffff7fca416 <+150>: jne    0x7ffff7fca3d8 ; <+88>
  0x7ffff7fca418 <+152>: movl   $0x2, %esi
  0x7ffff7fca41d <+157>: movq   %r12, %rdi
  0x7ffff7fca420 <+160>: movl   %ecx, -0x38(%rbp)
  0x7ffff7fca423 <+163>: movq   %rsp, -0x48(%rbp)
  0x7ffff7fca427 <+167>: callq  0x7ffff7fdf9e0 ; ___lldb_unnamed_symbol_1a9e0
  0x7ffff7fca42c <+172>: movl   -0x38(%rbp), %eax
  0x7ffff7fca42f <+175>: movq   %rsp, %rdx
  0x7ffff7fca432 <+178>: movq   %rax, %rcx
  0x7ffff7fca435 <+181>: leaq   0xf(,%rax,8), %rax
  0x7ffff7fca43d <+189>: shrq   $0x4, %rax
  0x7ffff7fca441 <+193>: shlq   $0x4, %rax
  0x7ffff7fca445 <+197>: movq   %rax, %rsi
  0x7ffff7fca448 <+200>: andq   $-0x1000, %rsi ; imm = 0xF000
  0x7ffff7fca44f <+207>: subq   %rsi, %rdx
  0x7ffff7fca452 <+210>: cmpq   %rdx, %rsp
  0x7ffff7fca455 <+213>: je     0x7ffff7fca46c ; <+236>
  0x7ffff7fca457 <+215>: subq   $0x1000, %rsp ; imm = 0x1000
  0x7ffff7fca45e <+222>: orq    $0x0, 0xff8(%rsp)
  0x7ffff7fca467 <+231>: cmpq   %rdx, %rsp
  0x7ffff7fca46a <+234>: jne    0x7ffff7fca457 ; <+215>
  0x7ffff7fca46c <+236>: andl   $0xfff, %eax ; imm = 0xFFF
  0x7ffff7fca471 <+241>: subq   %rax, %rsp
  0x7ffff7fca474 <+244>: testq  %rax, %rax
  0x7ffff7fca477 <+247>: jne    0x7ffff7fca5e9 ; <+617>
  0x7ffff7fca47d <+253>: movq   (%rbx), %rax
  0x7ffff7fca480 <+256>: movq   %rsp, %r8
  0x7ffff7fca483 <+259>: xorl   %r15d, %r15d
  0x7ffff7fca486 <+262>: testq  %rax, %rax
  0x7ffff7fca489 <+265>: jne    0x7ffff7fca499 ; <+281>
  0x7ffff7fca48b <+267>: jmp    0x7ffff7fca4ca ; <+330>
  0x7ffff7fca48d <+269>: nopl   (%rax)
  0x7ffff7fca490 <+272>: movq   0x18(%rax), %rax
  0x7ffff7fca494 <+276>: testq  %rax, %rax
  0x7ffff7fca497 <+279>: je     0x7ffff7fca4ca ; <+330>
  0x7ffff7fca499 <+281>: cmpq   %rax, 0x28(%rax)
  0x7ffff7fca49d <+285>: jne    0x7ffff7fca490 ; <+272>
  0x7ffff7fca49f <+287>: cmpl   %ecx, %r15d
  0x7ffff7fca4a2 <+290>: jae    0x7ffff7fca612 ; <+658>
  0x7ffff7fca4a8 <+296>: movl   %r15d, %edx
  0x7ffff7fca4ab <+299>: movq   %rax, (%r8,%rdx,8)
  0x7ffff7fca4af <+303>: movl   %r15d, 0x424(%rax)
  0x7ffff7fca4b6 <+310>: addl   $0x1, %r15d
  0x7ffff7fca4ba <+314>: addl   $0x1, 0x350(%rax)
  0x7ffff7fca4c1 <+321>: movq   0x18(%rax), %rax
  0x7ffff7fca4c5 <+325>: testq  %rax, %rax
  0x7ffff7fca4c8 <+328>: jne    0x7ffff7fca499 ; <+281>
  0x7ffff7fca4ca <+330>: cmpl   %r15d, %ecx
  0x7ffff7fca4cd <+333>: sete   %al
  0x7ffff7fca4d0 <+336>: testq  %r12, %r12
  0x7ffff7fca4d3 <+339>: jne    0x7ffff7fca4dd ; <+349>
  0x7ffff7fca4d5 <+341>: testb  %al, %al
  0x7ffff7fca4d7 <+343>: je     0x7ffff7fca650 ; <+720>
  0x7ffff7fca4dd <+349>: testq  %r12, %r12
  0x7ffff7fca4e0 <+352>: sete   %dl
  0x7ffff7fca4e3 <+355>: testb  %al, %al
  0x7ffff7fca4e5 <+357>: jne    0x7ffff7fca4f7 ; <+375>
  0x7ffff7fca4e7 <+359>: testb  %dl, %dl
  0x7ffff7fca4e9 <+361>: jne    0x7ffff7fca4f7 ; <+375>
  0x7ffff7fca4eb <+363>: subl   $0x1, %ecx
  0x7ffff7fca4ee <+366>: cmpl   %r15d, %ecx
  0x7ffff7fca4f1 <+369>: jne    0x7ffff7fca631 ; <+689>
  0x7ffff7fca4f7 <+375>: movq   %r8, %rdi
  0x7ffff7fca4fa <+378>: movzbl %dl, %edx
  0x7ffff7fca4fd <+381>: movl   $0x1, %ecx
  0x7ffff7fca502 <+386>: movl   %r15d, %esi
  0x7ffff7fca505 <+389>: movq   %r8, -0x38(%rbp)
  0x7ffff7fca509 <+393>: callq  0x7ffff7fd83a0 ; ___lldb_unnamed_symbol_133a0
  0x7ffff7fca50e <+398>: movq   %r13, %rdi
  0x7ffff7fca511 <+401>: callq  *0x32511(%rip)
  0x7ffff7fca517 <+407>: testl  %r15d, %r15d
  0x7ffff7fca51a <+410>: je     0x7ffff7fca54e ; <+462>
  0x7ffff7fca51c <+412>: movq   -0x38(%rbp), %r8
  0x7ffff7fca520 <+416>: movl   %r15d, %esi
  0x7ffff7fca523 <+419>: leaq   (%r8,%rsi,8), %rax
  0x7ffff7fca527 <+423>: nopw   (%rax,%rax)
  0x7ffff7fca530 <+432>: movq   (%r8), %r15
  0x7ffff7fca533 <+435>: testb  $0x10, 0x354(%r15)
  0x7ffff7fca53b <+443>: jne    0x7ffff7fca568 ; <+488>
  0x7ffff7fca53d <+445>: addq   $0x8, %r8
  0x7ffff7fca541 <+449>: subl   $0x1, 0x350(%r15)
  0x7ffff7fca549 <+457>: cmpq   %r8, %rax
  0x7ffff7fca54c <+460>: jne    0x7ffff7fca530 ; <+432>
  0x7ffff7fca54e <+462>: xorl   %esi, %esi
  0x7ffff7fca550 <+464>: movq   %r12, %rdi
  0x7ffff7fca553 <+467>: callq  0x7ffff7fdf9e0 ; ___lldb_unnamed_symbol_1a9e0
  0x7ffff7fca558 <+472>: movq   -0x48(%rbp), %rsp
  0x7ffff7fca55c <+476>: jmp    0x7ffff7fca3e1 ; <+97>
  0x7ffff7fca561 <+481>: nopl   (%rax)
  0x7ffff7fca568 <+488>: movq   %r15, %rdi
  0x7ffff7fca56b <+491>: movq   %rax, -0x40(%rbp)
  0x7ffff7fca56f <+495>: movq   %r8, -0x38(%rbp)
  0x7ffff7fca573 <+499>: callq  0x7ffff7fc6090 ; ___lldb_unnamed_symbol_1090
  0x7ffff7fca578 <+504>: movq   %r15, %rdi
  0x7ffff7fca57b <+507>: callq  0x7ffff7fdfcd0 ; ___lldb_unnamed_symbol_1acd0
  0x7ffff7fca580 <+512>: movq   -0x38(%rbp), %r8
  0x7ffff7fca584 <+516>: movq   -0x40(%rbp), %rax
  0x7ffff7fca588 <+520>: subl   $0x1, 0x350(%r15)
  0x7ffff7fca590 <+528>: addq   $0x8, %r8
  0x7ffff7fca594 <+532>: cmpq   %r8, %rax
  0x7ffff7fca597 <+535>: jne    0x7ffff7fca530 ; <+432>
  0x7ffff7fca599 <+537>: xorl   %esi, %esi
  0x7ffff7fca59b <+539>: movq   %r12, %rdi
  0x7ffff7fca59e <+542>: callq  0x7ffff7fdf9e0 ; ___lldb_unnamed_symbol_1a9e0
  0x7ffff7fca5a3 <+547>: movq   -0x48(%rbp), %rsp
  0x7ffff7fca5a7 <+551>: jmp    0x7ffff7fca3e1 ; <+97>
  0x7ffff7fca5ac <+556>: nopl   (%rax)
  0x7ffff7fca5b0 <+560>: cmpl   $0x1, -0x4c(%rbp)
  0x7ffff7fca5b4 <+564>: je     0x7ffff7fca5d1 ; <+593>
  0x7ffff7fca5b6 <+566>: movl   0x32894(%rip), %eax ; _rtld_global_ro + 944
  0x7ffff7fca5bc <+572>: movl   $0x1, -0x4c(%rbp)
  0x7ffff7fca5c3 <+579>: movl   $0x1, %r14d
  0x7ffff7fca5c9 <+585>: testl  %eax, %eax
  0x7ffff7fca5cb <+587>: jne    0x7ffff7fca3a6 ; <+38>
  0x7ffff7fca5d1 <+593>: testb  $-0x80, 0x324c8(%rip) ; _rtld_global_ro
  0x7ffff7fca5d8 <+600>: jne    0x7ffff7fca5f4 ; <+628>
  0x7ffff7fca5da <+602>: leaq   -0x28(%rbp), %rsp
  0x7ffff7fca5de <+606>: popq   %rbx
  0x7ffff7fca5df <+607>: popq   %r12
  0x7ffff7fca5e1 <+609>: popq   %r13
  0x7ffff7fca5e3 <+611>: popq   %r14
  0x7ffff7fca5e5 <+613>: popq   %r15
  0x7ffff7fca5e7 <+615>: popq   %rbp
  0x7ffff7fca5e8 <+616>: retq
  0x7ffff7fca5e9 <+617>: orq    $0x0, -0x8(%rsp,%rax)
  0x7ffff7fca5ef <+623>: jmp    0x7ffff7fca47d ; <+253>
  0x7ffff7fca5f4 <+628>: movq   0x334a5(%rip), %rdx ; _rtld_global + 2720
  0x7ffff7fca5fb <+635>: movq   0x33496(%rip), %rsi ; _rtld_global + 2712
  0x7ffff7fca602 <+642>: leaq   0x2a40f(%rip), %rdi
  0x7ffff7fca609 <+649>: xorl   %eax, %eax
  0x7ffff7fca60b <+651>: callq  0x7ffff7fd3b00 ; ___lldb_unnamed_symbol_eb00
  0x7ffff7fca610 <+656>: jmp    0x7ffff7fca5da ; <+602>
  0x7ffff7fca612 <+658>: leaq   0x2a477(%rip), %rcx
  0x7ffff7fca619 <+665>: movl   $0x4d, %edx
  0x7ffff7fca61e <+670>: leaq   0x2874d(%rip), %rsi
  0x7ffff7fca625 <+677>: leaq   0x28750(%rip), %rdi
  0x7ffff7fca62c <+684>: callq  0x7ffff7fe2370 ; ___lldb_unnamed_symbol_1d370
  0x7ffff7fca631 <+689>: leaq   0x2a458(%rip), %rcx
  0x7ffff7fca638 <+696>: movl   $0x58, %edx
  0x7ffff7fca63d <+701>: leaq   0x2872e(%rip), %rsi
  0x7ffff7fca644 <+708>: leaq   0x2a395(%rip), %rdi
  0x7ffff7fca64b <+715>: callq  0x7ffff7fe2370 ; ___lldb_unnamed_symbol_1d370
  0x7ffff7fca650 <+720>: leaq   0x2a439(%rip), %rcx
  0x7ffff7fca657 <+727>: movl   $0x57, %edx
  0x7ffff7fca65c <+732>: leaq   0x2870f(%rip), %rsi
  0x7ffff7fca663 <+739>: leaq   0x2a34e(%rip), %rdi
  0x7ffff7fca66a <+746>: callq  0x7ffff7fe2370 ; ___lldb_unnamed_symbol_1d370
  0x7ffff7fca66f <+751>: nop
```

## 附录 C：关键 LLDB 检查命令与结果索引

> 命令用于定位记录；所有绝对地址只对对应运行时映射成立。运行时变量会随重启改变，不要把老地址硬编码到新环境。

| 意图 | 已用命令 / 结果 |
|---|---|
| 查看调用者 | `bt` → `exit` / glibc 启动代码 / `_start` |
| 查退出回调链表头 | `<+70>` 停下后 `register read r15` → `0x7ffff7e05fc0` |
| 读当前节点 `idx` | `memory read --size 8 --format d --count 1`，地址表达式为 `` `$r15 + 8` ``；旧版 `5`、新版 `1` |
| 读旧版前五条回调记录 | `memory read --size 8 --format x --count 20`，地址表达式为 `` `$r15 + 0x10` `` |
| 读新版首条回调记录 | `memory read --size 8 --format x --count 4`，地址表达式为 `` `$r15 + 0x10` `` |
| 查看实际回调入口 | `<+356>` 停下后 `register read rax` → `0x7ffff7fca380` |
| 查目标映像 | `image lookup --address 0x00007ffff7fca380` → `ld-linux...+0x5380` |
| 确认回调已返回 | `<+358>` 的断点在 `main-debug` 中实际命中 |
| 确认链表回收分支 | `<+456>` 实际命中 |
| 确认最终收尾 | `<+256>` 实际命中；`<+607>` 实际命中 |
| 锁竞争分支 | `<+617>` 断点未命中 |
| 结束状态 | `main-debug`：`Process ... exited with status = 0` |

### 备份时仍值得补存的外部证据（并非本次已做）

- `getconf GNU_LIBC_VERSION`：记录 libc 的精确运行库版本。
- `ldd ./archive/pre-cmake-redesign/main` 或对应 `readelf -d`：帮助识别旧二进制的动态依赖；**注意** `ldd` 的解释依赖当前环境。
- 对归档的可执行文件核对 SHA-256 与 Build ID；若需要长期逐字节复现 glibc 内部汇编，还应保存对应 `libc.so.6`、`ld-linux-x86-64.so.2` 的版本标识或文件。

**归档状态：本次 `call main` 后的退出主线已经闭合；原始观测与未确认推断已分离。**
