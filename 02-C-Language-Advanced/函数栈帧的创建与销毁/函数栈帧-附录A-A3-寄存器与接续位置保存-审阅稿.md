# 附录 A · 第三段：把接续需要的机器状态写入保存区

`_setjmp` 已经准备好实参，并通过 `jmp` 进入 `__sigsetjmp`。从这里开始，前面预留的局部空间将逐项写入实际数据：先记录需要保留的寄存器，再保存将来接续时应使用的栈位置和代码位置。

## 先把结构体布局放在手边

为便于对照，可以把 glibc 的实际布局展开成下面这份 C 风格声明。成员名称按本节的解释重新拟定，偏移按本次 x86-64、64 位指针的布局标出；源码中的数组和多层私有成员访问则作了展开。

```c
#include <stdint.h>

struct ResumePointState {
    /* 接续执行所需的机器状态 */

    /* +0x00 */ uint64_t saved_rbx;          // 保存 rbx 的原值
    /* +0x08 */ uint64_t saved_rbp_mangled;  // 保存混淆后的 rbp，本次为栈帧基准
    /* +0x10 */ uint64_t saved_r12;          // 保存 r12 的原值
    /* +0x18 */ uint64_t saved_r13;          // 保存 r13 的原值
    /* +0x20 */ uint64_t saved_r14;          // 保存 r14 的原值
    /* +0x28 */ uint64_t saved_r15;          // 保存 r15 的原值
    /* +0x30 */ uint64_t resume_rsp_mangled; // 接续时应使用的 rsp，经过混淆
    /* +0x38 */ uint64_t resume_pc_mangled;  // 接续执行的代码地址，经过混淆

    /* +0x40 */ int32_t mask_was_saved;       // 是否保存信号阻塞集，后面本次写入 0
    /* +0x44 */ uint8_t alignment_padding[4]; // 对齐填充

    /* 线程恢复节点的管理信息，稍后由运行库填写 */

    /* +0x48 */ struct ResumePointState *prev; // 此前登记的恢复节点
    /* +0x50 */ void *saved_cleanup_head;      // 登记时的旧式清理链表头
    /* +0x58 */ uint8_t private_tail[16];      // 剩余私有空间，结合具体路径解释
};

/* 本次布局：对齐为 8 字节，总大小为 0x68，即 104 字节。 */
```

这里展示的是外层 `pthread_unwind_buf` 的展开布局。本段主要填写前八个机器状态槽位；信号阻塞集保存标志与节点关联信息，要在后面才处理。声明中的 `private_tail` 保留原始空间形式：外层源码的私有区包含取消类型等信息，而后面可选的影子栈保存又会使用其中的空间，不能提前将整块区域解释为一个独占的影子栈成员。

本次 `%rdi` 指向 `0x7fffffffe080`。因此，`0x8(%rdi)` 就对应 `saved_rbp_mangled`，`0x30(%rdi)` 对应 `resume_rsp_mangled`。结构体里的偏移与接下来看到的寻址偏移，可以一一对应。

## 进入公共保存逻辑，记录 rbx

```asm
0x7ffff7c44ed0 <+0>: endbr64
0x7ffff7c44ed4 <+4>: movq   %rbx, (%rdi)
```

![可落点标记_3.png](https://img.mdwind.online/img/%E5%8F%AF%E8%90%BD%E7%82%B9%E6%A0%87%E8%AE%B0_3.png)

![可落点标记_4.png](https://img.mdwind.online/img/%E5%8F%AF%E8%90%BD%E7%82%B9%E6%A0%87%E8%AE%B0_4.png)

当前先执行入口的 `endbr64`，它的作用上一段已经讨论。`_setjmp` 是通过直接 `jmp` 来到这里的，沿途没有压入新的返回地址，也没有另外建立栈帧。当前 `%rsp` 仍为 `0x7fffffffe058`，`%rbp` 仍为运行库调用者的 `0x7fffffffe0f0`。

下一条 `movq` 把 `%rbx` 的八字节原值写入 `%rdi` 指向的位置，即保存区的 `+0x00`。

![暂存上级调用者寄存器中的有效值_1.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E4%B8%8A%E7%BA%A7%E8%B0%83%E7%94%A8%E8%80%85%E5%AF%84%E5%AD%98%E5%99%A8%E4%B8%AD%E7%9A%84%E6%9C%89%E6%95%88%E5%80%BC_1.png)

![暂存上级调用者寄存器中的有效值_2.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E4%B8%8A%E7%BA%A7%E8%B0%83%E7%94%A8%E8%80%85%E5%AF%84%E5%AD%98%E5%99%A8%E4%B8%AD%E7%9A%84%E6%9C%89%E6%95%88%E5%80%BC_2.png)

本次 `%rbx` 的值为 `0x7fffffffe178`，与 `argv` 数组地址相对应。`<+4>` 将这个指针值写入保存区的 `+0x00` 槽位。

在本平台的调用约定中，`rbx`、`rbp`、`r12`～`r15` 承担着保留跨调用数据的职责。调用者可以将后续仍需使用的数据放入这些寄存器，并依赖它们在普通函数调用返回后保持原值。非局部恢复也利用了这项约定：保存点记录它们的值，恢复时重新装回，使接续点之后的代码仍能使用原先的数据。

这些寄存器具有明确的保留职责，其中数据的具体意义则由使用它们的代码决定。`setjmp` 只记录寄存器的当前值，不解释它们存放的是启动参数、地址还是其他中间结果。各个槽位按照寄存器命名，正好与汇编中的逐项写入相对应。

## 保存 rbp：先复制，再混淆，最后写入

```asm
0x7ffff7c44ed7 <+7>:  movq %rbp, %rax
0x7ffff7c44eda <+10>: xorq %fs:0x30, %rax
0x7ffff7c44ee3 <+19>: rolq $0x11, %rax
0x7ffff7c44ee7 <+23>: movq %rax, 0x8(%rdi)
```

`<+7>` 先把 `%rbp` 的值复制到临时寄存器 `%rax`。本次复制的是 `0x7fffffffe0f0`，正在使用的 `%rbp` 本身保持不变。

![暂存恢复时的栈基址_1.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E6%81%A2%E5%A4%8D%E6%97%B6%E7%9A%84%E6%A0%88%E5%9F%BA%E5%9D%80_1.png)

![暂存恢复时的栈基址_2.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E6%81%A2%E5%A4%8D%E6%97%B6%E7%9A%84%E6%A0%88%E5%9F%BA%E5%9D%80_2.png)

接下来的 `xorq` 从 `%fs:0x30` 读取指针保护值，与 `%rax` 异或；`rolq $0x11` 则把所得的 64 位数循环左旋 17 位。这里的 `0x11` 是十六进制，换成十进制就是 17。

![栈基址指针混淆_1.png](https://img.mdwind.online/img/%E6%A0%88%E5%9F%BA%E5%9D%80%E6%8C%87%E9%92%88%E6%B7%B7%E6%B7%86_1.png)

![栈基址指针混淆_2.png](https://img.mdwind.online/img/%E6%A0%88%E5%9F%BA%E5%9D%80%E6%8C%87%E9%92%88%E6%B7%B7%E6%B7%86_2.png)

循环移位与丢弃高位的普通左移不同：从高位移出去的比特会回到低位，信息没有丢失。因此，这个变换是可逆的。若用 `G` 表示指针保护值、`P` 表示原值，保存过程可以写为：

```text
保存值 = 循环左旋64位(P XOR G, 17)
原值   = 循环右旋64位(保存值, 17) XOR G
```

固定的旋转次数本身不是秘密；异或与循环移位共同构成这里的指针混淆，恢复方按相反顺序处理。

![循环移位混淆后的栈基址_1.png](https://img.mdwind.online/img/%E5%BE%AA%E7%8E%AF%E7%A7%BB%E4%BD%8D%E6%B7%B7%E6%B7%86%E5%90%8E%E7%9A%84%E6%A0%88%E5%9F%BA%E5%9D%80_1.png)

![循环移位混淆后的栈基址_2.png](https://img.mdwind.online/img/%E5%BE%AA%E7%8E%AF%E7%A7%BB%E4%BD%8D%E6%B7%B7%E6%B7%86%E5%90%8E%E7%9A%84%E6%A0%88%E5%9F%BA%E5%9D%80_2.png)

混淆完成后，本次 `%rax` 的值为 `0x40bbe00ab5d8be57`。最后的 `<+23>` 才把这个值写入 `%rdi+0x08`，也就是 `0x7fffffffe088`。

![将恢复时所需栈基址写到上下文维护节点_1.png](https://img.mdwind.online/img/%E5%B0%86%E6%81%A2%E5%A4%8D%E6%97%B6%E6%89%80%E9%9C%80%E6%A0%88%E5%9F%BA%E5%9D%80%E5%86%99%E5%88%B0%E4%B8%8A%E4%B8%8B%E6%96%87%E7%BB%B4%E6%8A%A4%E8%8A%82%E7%82%B9_1.png)

![将恢复时所需栈基址写到上下文维护节点_2.png](https://img.mdwind.online/img/%E5%B0%86%E6%81%A2%E5%A4%8D%E6%97%B6%E6%89%80%E9%9C%80%E6%A0%88%E5%9F%BA%E5%9D%80%E5%86%99%E5%88%B0%E4%B8%8A%E4%B8%8B%E6%96%87%E7%BB%B4%E6%8A%A4%E8%8A%82%E7%82%B9_2.png)

图中箭头仍指着写入指令，表示此刻结果已经在 `%rax` 中准备好，目标槽位尚未由这条指令写入。执行后，`saved_rbp_mangled` 才具有这个混淆值。后面的图也沿用同样的“当前待执行指令”表示方式。

### 为什么对这些值进行混淆

这里保护的是保存区中的副本。它们暂时作为普通数据放在内存里，真正恢复时却会决定使用哪一片栈空间，以及从哪里继续执行。攻击者如果能够定向改写这些保存值，内存破坏就可能进一步发展成对执行流的控制，造成信息泄露、数据篡改等后果。

直接保存原始地址时，攻击者可能尝试把自己选定的地址写入相应槽位。经过混淆后，恢复方还要进行反向变换；直接写入目标地址，恢复得到的不会就是那个目标。攻击者需要构造与保护值相匹配的保存值，才能准确得到预期的执行去向。

指针混淆在内存破坏与定向控制执行流之间增加了一道障碍。攻击者即使获得对保存区的写入能力，也还需要解决混淆值的构造问题，才能使程序沿选定的路径继续执行。盲目覆盖可能得到无效地址并使程序崩溃；保护信息泄露或更强的读写能力，则可能削弱这项措施的效果。

这是一项降低漏洞利用机会的加固措施。它提高了将关键保存值改造成可控执行目标的难度，不能据此保证所有篡改最终都会导致终止。

本次使用的指针保护值位于 `%fs:0x30`；前面读取的栈金丝雀位于 `%fs:0x28`，二者分别用于对应的保护机制。

## 连续保存 r12～r15

```asm
0x7ffff7c44eeb <+27>: movq %r12, 0x10(%rdi)
0x7ffff7c44eef <+31>: movq %r13, 0x18(%rdi)
0x7ffff7c44ef3 <+35>: movq %r14, 0x20(%rdi)
0x7ffff7c44ef7 <+39>: movq %r15, 0x28(%rdi)
```

接下来的四条指令，分别把四个寄存器的原值写入各自的槽位。它们延续了前面保存 `%rbx` 的原则，代码不会解释其中的数据含义，也没有根据当前值临时挑选槽位。

下面四组图依次对应各条写入尚未执行时的状态。随着单步推进，已经写入的槽位会保留下来。

![暂存调用者存储有效值的寄存器_10.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E8%B0%83%E7%94%A8%E8%80%85%E5%AD%98%E5%82%A8%E6%9C%89%E6%95%88%E5%80%BC%E7%9A%84%E5%AF%84%E5%AD%98%E5%99%A8_10.png)

![暂存调用者存储有效值的寄存器_11.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E8%B0%83%E7%94%A8%E8%80%85%E5%AD%98%E5%82%A8%E6%9C%89%E6%95%88%E5%80%BC%E7%9A%84%E5%AF%84%E5%AD%98%E5%99%A8_11.png)

![暂存调用者存储有效值的寄存器_20.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E8%B0%83%E7%94%A8%E8%80%85%E5%AD%98%E5%82%A8%E6%9C%89%E6%95%88%E5%80%BC%E7%9A%84%E5%AF%84%E5%AD%98%E5%99%A8_20.png)

![暂存调用者存储有效值的寄存器_21.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E8%B0%83%E7%94%A8%E8%80%85%E5%AD%98%E5%82%A8%E6%9C%89%E6%95%88%E5%80%BC%E7%9A%84%E5%AF%84%E5%AD%98%E5%99%A8_21.png)

![暂存调用者存储有效值的寄存器_30.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E8%B0%83%E7%94%A8%E8%80%85%E5%AD%98%E5%82%A8%E6%9C%89%E6%95%88%E5%80%BC%E7%9A%84%E5%AF%84%E5%AD%98%E5%99%A8_30.png)

![暂存调用者存储有效值的寄存器_31.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E8%B0%83%E7%94%A8%E8%80%85%E5%AD%98%E5%82%A8%E6%9C%89%E6%95%88%E5%80%BC%E7%9A%84%E5%AF%84%E5%AD%98%E5%99%A8_31.png)

![暂存调用者存储有效值的寄存器_40.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E8%B0%83%E7%94%A8%E8%80%85%E5%AD%98%E5%82%A8%E6%9C%89%E6%95%88%E5%80%BC%E7%9A%84%E5%AF%84%E5%AD%98%E5%99%A8_40.png)

![暂存调用者存储有效值的寄存器_41.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E8%B0%83%E7%94%A8%E8%80%85%E5%AD%98%E5%82%A8%E6%9C%89%E6%95%88%E5%80%BC%E7%9A%84%E5%AF%84%E5%AD%98%E5%99%A8_41.png)

四条指令依次填写 `saved_r12`～`saved_r15`。加上前面保存的 `rbx` 和 `rbp`，六个被调用者保存寄存器的值都已记录在保存区中。槽位偏移由本次实现确定，恢复方按照相同的位置读回。

原图标出了这些寄存器本次的数值及相关用途。这里保存的仍是寄存器状态；例如 `%r12` 此刻保存着 `argc` 的副本，而 `saved_r12` 的固定职责是记录 `%r12`。保存函数不需要了解这个值在启动过程中的具体含义。

这一组寄存器直接按原值写入。对 `%rbp`、接续栈位置与接续代码地址，glibc 则采用前面看到的混淆处理。这些处理方式由保存项目确定，与某次运行中寄存器的值是否恰好是指针无关。

## 保存接续以后应使用的 rsp

```asm
0x7ffff7c44efb <+43>: leaq 0x8(%rsp), %rdx
0x7ffff7c44f00 <+48>: xorq %fs:0x30, %rdx
0x7ffff7c44f09 <+57>: rolq $0x11, %rdx
0x7ffff7c44f0d <+61>: movq %rdx, 0x30(%rdi)
```

当前 `%rsp` 为 `0x7fffffffe058`，该位置保存着最初 `callq _setjmp` 压入的返回地址。`<+43>` 使用 `leaq` 计算 `%rsp+8`，得到 `0x7fffffffe060`，没有读取这个地址中的内容。

![暂存恢复后应当处于的rsp地址_1.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E6%81%A2%E5%A4%8D%E5%90%8E%E5%BA%94%E5%BD%93%E5%A4%84%E4%BA%8E%E7%9A%84rsp%E5%9C%B0%E5%9D%80_1.png)

![暂存恢复后应当处于的rsp地址_2.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E6%81%A2%E5%A4%8D%E5%90%8E%E5%BA%94%E5%BD%93%E5%A4%84%E4%BA%8E%E7%9A%84rsp%E5%9C%B0%E5%9D%80_2.png)

为什么要加八？因为将来接续到保存调用之后，应当具有“这次调用已经返回”的栈状态。正常的 `retq` 取走八字节返回地址后，栈指针就会从 `0x7fffffffe058` 回到 `0x7fffffffe060`。这里提前记录的正是那个返回以后的位置。

这时，变化的是临时寄存器 `%rdx`，实际 `%rsp` 仍指向 `0x7fffffffe058`。保存代码没有提前弹出返回地址，也没有改变当前栈顶。

随后，用同一个指针保护值异或，并循环左旋 17 位。

![指针混淆  rsp _1.png](https://img.mdwind.online/img/%E6%8C%87%E9%92%88%E6%B7%B7%E6%B7%86%20%20rsp%20_1.png)

![指针混淆  rsp _2.png](https://img.mdwind.online/img/%E6%8C%87%E9%92%88%E6%B7%B7%E6%B7%86%20%20rsp%20_2.png)

![循环移位 rsp _1.png](https://img.mdwind.online/img/%E5%BE%AA%E7%8E%AF%E7%A7%BB%E4%BD%8D%20rsp%20_1.png)

![循环移位 rsp _2.png](https://img.mdwind.online/img/%E5%BE%AA%E7%8E%AF%E7%A7%BB%E4%BD%8D%20rsp%20_2.png)

本次混淆结果为 `0x40bbe00ab4f8be57`。执行 `<+61>` 后，它被写入 `%rdi+0x30`，即 `0x7fffffffe0b0`，对应 `resume_rsp_mangled`。

![rsp 写入上下文恢复节点_1.png](https://img.mdwind.online/img/rsp%20%E5%86%99%E5%85%A5%E4%B8%8A%E4%B8%8B%E6%96%87%E6%81%A2%E5%A4%8D%E8%8A%82%E7%82%B9_1.png)

![rsp 写入上下文恢复节点_2.png](https://img.mdwind.online/img/rsp%20%E5%86%99%E5%85%A5%E4%B8%8A%E4%B8%8B%E6%96%87%E6%81%A2%E5%A4%8D%E8%8A%82%E7%82%B9_2.png)

这个成员表示恢复后要装入 `%rsp` 的值，而不是保存时 `%rsp` 的原样副本。理解它的语义，比只记住“这里存了一个栈指针”更重要。

## 从栈顶取出接续代码地址

```asm
0x7ffff7c44f11 <+65>: movq (%rsp), %rax
0x7ffff7c44f15 <+69>: nop
0x7ffff7c44f16 <+70>: xorq %fs:0x30, %rax
0x7ffff7c44f1f <+79>: rolq $0x11, %rax
0x7ffff7c44f23 <+83>: movq %rax, 0x38(%rdi)
```

`<+65>` 换成了 `movq (%rsp),%rax`。这次确实读取内存，取出当前栈顶存储的八字节返回地址：`0x7ffff7c2a181`，也就是运行库调用者的 `<+49>`。

![暂存恢复后应当处于的rip地址_1.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E6%81%A2%E5%A4%8D%E5%90%8E%E5%BA%94%E5%BD%93%E5%A4%84%E4%BA%8E%E7%9A%84rip%E5%9C%B0%E5%9D%80_1.png)

![暂存恢复后应当处于的rip地址_2.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E6%81%A2%E5%A4%8D%E5%90%8E%E5%BA%94%E5%BD%93%E5%A4%84%E4%BA%8E%E7%9A%84rip%E5%9C%B0%E5%9D%80_2.png)

它没有读取当前 `%rip`。当前正在执行的是保存函数内部的指令，而将来需要接续的是原始调用之后的位置；那个位置已经由 `callq` 压在栈顶，直接取出即可。

在两条混淆指令之前，还夹着一条 `nop`。

![辅助工具探针标记_1.png](https://img.mdwind.online/img/%E8%BE%85%E5%8A%A9%E5%B7%A5%E5%85%B7%E6%8E%A2%E9%92%88%E6%A0%87%E8%AE%B0_1.png)

![辅助工具探针标记_2.png](https://img.mdwind.online/img/%E8%BE%85%E5%8A%A9%E5%B7%A5%E5%85%B7%E6%8E%A2%E9%92%88%E6%A0%87%E8%AE%B0_2.png)

这条 `nop` 保持当前寄存器和内存状态。对照 glibc 的 `setjmp.S`，读取返回地址与混淆返回地址之间设置了 `LIBC_PROBE(setjmp, ...)` 静态探针，本次指令的位置与之对应。静态探针为跟踪工具提供预先安排的观察位置；没有工具使用它时，程序照常经过这里，继续执行后面的混淆操作。具体对应关系还可以通过实验二进制中的探针元数据进一步核对。

随后，返回地址同样经过异或与循环左旋。

![指针混淆  rip _1.png](https://img.mdwind.online/img/%E6%8C%87%E9%92%88%E6%B7%B7%E6%B7%86%20%20rip%20_1.png)

![指针混淆  rip _2.png](https://img.mdwind.online/img/%E6%8C%87%E9%92%88%E6%B7%B7%E6%B7%86%20%20rip%20_2.png)

![循环移位 rip _1.png](https://img.mdwind.online/img/%E5%BE%AA%E7%8E%AF%E7%A7%BB%E4%BD%8D%20rip%20_1.png)

![循环移位 rip _2.png](https://img.mdwind.online/img/%E5%BE%AA%E7%8E%AF%E7%A7%BB%E4%BD%8D%20rip%20_2.png)

本次得到的混淆值为 `0x40bbf070373abe57`，最后由 `<+83>` 写入 `%rdi+0x38`，即 `0x7fffffffe0b8`，对应 `resume_pc_mangled`。

![rip 写入上下文恢复节点_1.png](https://img.mdwind.online/img/rip%20%E5%86%99%E5%85%A5%E4%B8%8A%E4%B8%8B%E6%96%87%E6%81%A2%E5%A4%8D%E8%8A%82%E7%82%B9_1.png)

![rip 写入上下文恢复节点_2.png](https://img.mdwind.online/img/rip%20%E5%86%99%E5%85%A5%E4%B8%8A%E4%B8%8B%E6%96%87%E6%81%A2%E5%A4%8D%E8%8A%82%E7%82%B9_2.png)

这组图处于最后写入之前：结果已经在 `%rax` 中，`+0x38` 槽位尚未写入。执行该指令以后，本段的八个机器状态槽位全部填写完毕。栈顶原来保存的返回地址仍然存在，没有被这里的复制或混淆修改。

## 对照已经写入的布局

执行完 `<+83>` 后，本段保存的内容可以归纳如下。表中“原值”是混淆前的含义，不应与内存里实际存放的混淆结果混为一谈。

| 偏移 | 本次槽位地址 | 成员 | 保存内容 |
| --- | --- | --- | --- |
| `+0x00` | `0x7fffffffe080` | `saved_rbx` | `%rbx` 原值，本次为 `0x7fffffffe178` |
| `+0x08` | `0x7fffffffe088` | `saved_rbp_mangled` | 原值 `0x7fffffffe0f0`，保存为 `0x40bbe00ab5d8be57` |
| `+0x10` | `0x7fffffffe090` | `saved_r12` | `%r12` 原值 |
| `+0x18` | `0x7fffffffe098` | `saved_r13` | `%r13` 原值 |
| `+0x20` | `0x7fffffffe0a0` | `saved_r14` | `%r14` 原值 |
| `+0x28` | `0x7fffffffe0a8` | `saved_r15` | `%r15` 原值 |
| `+0x30` | `0x7fffffffe0b0` | `resume_rsp_mangled` | 原值 `0x7fffffffe060`，保存为 `0x40bbe00ab4f8be57` |
| `+0x38` | `0x7fffffffe0b8` | `resume_pc_mangled` | 原值 `0x7ffff7c2a181`，保存为 `0x40bbf070373abe57` |

其中两条取值指令尤其值得并列看：

```asm
leaq 0x8(%rsp), %rdx  # 计算接续后应使用的栈位置
movq (%rsp), %rax     # 读取接续执行的代码地址
```

它们分别回答“栈从哪里继续用”和“代码从哪里继续执行”。恢复方以后装回必要寄存器与目标栈位置，再转移到目标代码，就能够建立保存调用已经返回的接续状态。

截至这里，完成的是机器状态保存。后面仍有可选的影子栈处理、信号阻塞集保存标志，以及返回调用者后的节点登记，这些工作将在接下来的段落中继续完成。

---

### 材料与实现依据

插图按原静态图中的执行顺序排列，共十九组、三十八张；每组两张展示同一状态下不同范围的栈空间。图中箭头指向待执行指令。地址和混淆结果沿用原图，源码仅用于说明布局及操作含义。

- [glibc：pthread_unwind_buf 的外层定义](https://codebrowser.dev/glibc/glibc/nptl/descr.h.html)
- [glibc：x86-64 保存槽位定义](https://codebrowser.dev/glibc/glibc/sysdeps/x86_64/jmpbuf-offsets.h.html)
- [glibc：保存寄存器、接续位置与探针的实现](https://github.com/bminor/glibc/blob/master/sysdeps/x86_64/setjmp.S)
- [glibc：指针混淆与反向处理](https://codebrowser.dev/glibc/glibc/sysdeps/unix/sysv/linux/x86_64/pointer_guard.h.html)
- [glibc：恢复机器状态的实现](https://codebrowser.dev/glibc/glibc/sysdeps/x86_64/__longjmp.S.html)
