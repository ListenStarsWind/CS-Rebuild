# 附录 A · 第四段：影子栈、保存标志与第一次返回

前八个机器状态槽位已经写入，接续所需的普通寄存器、栈位置和代码位置都有了记录。接下来，`__sigsetjmp` 检查是否还需要保存影子栈指针，再进入后续函数处理信号阻塞集的保存标志，最后返回运行库调用者。

## 普通栈之外，为什么还有一份返回记录

```asm
0x7ffff7c44f27 <+87>:  testl  $0x2, %fs:0x48
0x7ffff7c44f33 <+99>:  je     0x7ffff7c44f3e ; <+110>
0x7ffff7c44f35 <+101>: rdsspq %rax
0x7ffff7c44f3a <+106>: movq   %rax, 0x58(%rdi)
0x7ffff7c44f3e <+110>: jmp    0x7ffff7c44f50
```

![检查是否启用影子栈的标记位_1.png](https://img.mdwind.online/img/%E6%A3%80%E6%9F%A5%E6%98%AF%E5%90%A6%E5%90%AF%E7%94%A8%E5%BD%B1%E5%AD%90%E6%A0%88%E7%9A%84%E6%A0%87%E8%AE%B0%E4%BD%8D_1.png)

![检查是否启用影子栈的标记位_2.png](https://img.mdwind.online/img/%E6%A3%80%E6%9F%A5%E6%98%AF%E5%90%A6%E5%90%AF%E7%94%A8%E5%BD%B1%E5%AD%90%E6%A0%88%E7%9A%84%E6%A0%87%E8%AE%B0%E4%BD%8D_2.png)

这里涉及 Intel CET 的另一项机制：影子栈（Shadow Stack）。前面讨论的 IBT 检查间接调用和跳转的目标位置，影子栈则维护函数调用的返回关系。

普通调用中，`call` 把返回地址压入普通栈；启用影子栈保护后，处理器还会在影子栈中保存一份返回地址。执行 `ret` 时，处理器检查两份返回地址是否一致。不一致，就触发控制保护异常。

这相当于在普通栈之外保留了一份受保护的返回记录。普通栈中的返回地址若被改写，受保护的另一份仍可用于返回检查。影子栈页面具有专门的访问保护，普通内存写指令无法像改写普通栈那样随意修改其中的记录。

普通栈除了返回地址，还存放局部变量、暂存寄存器等数据。影子栈主要维护返回记录，两者推进的步幅和使用方式不同，需要各自的指针：普通栈使用 `%rsp`，影子栈使用 SSP。这里的 `rdsspq` 就是读取影子栈指针的指令。

### 非局部恢复怎样影响两套栈

设想外层函数保存了接续点，之后依次调用中间函数和内部函数。正常返回时，每次 `ret` 都使用并撤去当前一层的返回记录，两套栈随着调用与返回保持对应。

如果内部函数发起非局部跳转，普通栈位置被直接恢复到外层，控制流也接回外层的代码，中间那些普通返回便被跨过了。影子栈若还停留在较深调用对应的位置，后续返回就可能出现错位。

| 时刻 | 普通栈及执行位置 | 影子栈 |
| --- | --- | --- |
| 内部函数发起恢复前 | 对应内部调用层次 | 维护当前调用链的返回记录 |
| 只恢复普通栈并跳回外层 | 已接回外层 | 仍可能停在被跨过的调用层次 |
| 外层随后沿原调用关系返回 | 取出外层应使用的返回地址 | 取出的记录可能属于另一层调用 |

间接 `jmp` 只转移执行流，不会像 `ret` 那样消费对应的影子栈返回记录。普通 `longjmp` 接续后，程序还可能继续业务逻辑，并在以后从外层函数正常返回，所以恢复实现需要协调影子栈位置。

glibc 的通用 x86-64 恢复实现中，可以看到读取保存的 SSP、使用 `rstorssp` 或 `incsspq` 等专门指令进行调整的逻辑。保存 SSP 时尚处于 `setjmp` 调用内部，接续目标却是调用已经返回的位置；源码还专门计入了越过这一次保存调用所需的额外调整。它与上一段普通栈保存 `%rsp+8` 的考虑相呼应。

线程退出则有不同的后续执行方式。这里涉及的 glibc 取消专用恢复路径，通过内部 `__longjmp_cancel` 恢复必要机器状态，而不恢复 SSP。接续后执行的是退出收尾，最后结束线程，不再通过被跨过的旧调用帧逐层返回。

这时，旧影子栈记录不再用于完成那些旧调用的返回。收尾代码仍然可以调用其他函数：新的 `call` 会建立新的返回记录，相应的 `ret` 使用这些新记录，结束后继续收尾。关键在于，它最终不会再沿旧调用链返回。

所以，普通非局部跳转与线程退出是否需要恢复这部分状态，取决于接续后怎样执行。通用恢复要为后续正常返回准备一致的返回关系；取消专用路径则利用退出过程不再通过旧调用帧返回的特点，只恢复其所需的状态。这也与上一段私有空间可以复用的布局相联系。

## 本次跳过 SSP 保存

`<+87>` 使用 `testl` 检查 `%fs:0x48` 中的相应特征位。立即数 `0x2` 对应其中的一位，按位与的结果只用于更新标志位，不写回内存。在本次 glibc 布局中，这里检查的是影子栈启用标记。

![未启用 跳过影子栈快照保存逻辑_1.png](https://img.mdwind.online/img/%E6%9C%AA%E5%90%AF%E7%94%A8%20%E8%B7%B3%E8%BF%87%E5%BD%B1%E5%AD%90%E6%A0%88%E5%BF%AB%E7%85%A7%E4%BF%9D%E5%AD%98%E9%80%BB%E8%BE%91_1.png)

![未启用 跳过影子栈快照保存逻辑_2.png](https://img.mdwind.online/img/%E6%9C%AA%E5%90%AF%E7%94%A8%20%E8%B7%B3%E8%BF%87%E5%BD%B1%E5%AD%90%E6%A0%88%E5%BF%AB%E7%85%A7%E4%BF%9D%E5%AD%98%E9%80%BB%E8%BE%91_2.png)

本次该位没有置位，结果为零，ZF 为 1，`<+99>` 的 `je` 跳到 `<+110>`，越过 `rdsspq` 和后面的写入。

![调用信号掩码保存逻辑_1.png](https://img.mdwind.online/img/%E8%B0%83%E7%94%A8%E4%BF%A1%E5%8F%B7%E6%8E%A9%E7%A0%81%E4%BF%9D%E5%AD%98%E9%80%BB%E8%BE%91_1.png)

![调用信号掩码保存逻辑_2.png](https://img.mdwind.online/img/%E8%B0%83%E7%94%A8%E4%BF%A1%E5%8F%B7%E6%8E%A9%E7%A0%81%E4%BF%9D%E5%AD%98%E9%80%BB%E8%BE%91_2.png)

若进入保存分支，`rdsspq %rax` 会读取 SSP，随后将其写入 `%rdi+0x58`。按本次保存区起点 `0x7fffffffe080` 计算，这个位置是 `0x7fffffffe0d8`，对应前面结构体的 `private_tail` 起点。

这八字节与外层私有区域存在空间复用；在取消专用路径中，恢复方并不使用这份 SSP 记录。本次分支直接跳过，`+0x58` 没有由这两条指令写入。

影子栈保存属于公共保存实现的可选能力。这里继续通过 `jmp` 进入下一部分，原先栈顶的返回地址仍保持不变。

## 进入后续函数，建立自己的栈帧

```asm
0x7ffff7c44f50 <+0>:  endbr64
0x7ffff7c44f54 <+4>:  pushq %rbp
0x7ffff7c44f55 <+5>:  movq  %rsp, %rbp
0x7ffff7c44f58 <+8>:  pushq %rbx
0x7ffff7c44f59 <+9>:  movq  %rdi, %rbx
0x7ffff7c44f5c <+12>: subq  $0x8, %rsp
```

这部分与 glibc 的 `__sigjmp_save` 实现相对应。原始调试器没有解析出其内部名称，图中保留了对应的匿名符号名称。

![可落点标记_5.png](https://img.mdwind.online/img/%E5%8F%AF%E8%90%BD%E7%82%B9%E6%A0%87%E8%AE%B0_5.png)

![可落点标记_6.png](https://img.mdwind.online/img/%E5%8F%AF%E8%90%BD%E7%82%B9%E6%A0%87%E8%AE%B0_6.png)

进入时，`%rsp=0x7fffffffe058`，栈顶仍存着最初 `callq _setjmp` 压入的 `0x7ffff7c2a181`。前面经过的两次 `jmp` 都没有增加新的返回地址。

执行入口标记后，先用 `pushq %rbp` 保存调用者的栈帧基准。

![压入  rbp_1.png](https://img.mdwind.online/img/%E5%8E%8B%E5%85%A5%20%20rbp_1.png)

![压入   rbp_2.png](https://img.mdwind.online/img/%E5%8E%8B%E5%85%A5%20%20%20rbp_2.png)

`%rsp` 从 `0x7fffffffe058` 减少八字节，变为 `0x7fffffffe050`，该位置写入旧 `%rbp`，即 `0x7fffffffe0f0`。随后将当前 `%rsp` 复制给 `%rbp`，建立本层的栈帧基准。

![建立     栈基址_1.png](https://img.mdwind.online/img/%E5%BB%BA%E7%AB%8B%20%20%20%20%20%E6%A0%88%E5%9F%BA%E5%9D%80_1.png)

![建立     栈基址_2.png](https://img.mdwind.online/img/%E5%BB%BA%E7%AB%8B%20%20%20%20%20%E6%A0%88%E5%9F%BA%E5%9D%80_2.png)

接下来，当前函数要用 `%rbx` 暂存保存区地址，所以先把旧 `%rbx` 压栈。

![暂存上级调用者可能使用的寄存器_1.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E4%B8%8A%E7%BA%A7%E8%B0%83%E7%94%A8%E8%80%85%E5%8F%AF%E8%83%BD%E4%BD%BF%E7%94%A8%E7%9A%84%E5%AF%84%E5%AD%98%E5%99%A8_1.png)

![暂存上级调用者可能使用的寄存器_2.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E4%B8%8A%E7%BA%A7%E8%B0%83%E7%94%A8%E8%80%85%E5%8F%AF%E8%83%BD%E4%BD%BF%E7%94%A8%E7%9A%84%E5%AF%84%E5%AD%98%E5%99%A8_2.png)

执行后，`%rsp=0x7fffffffe048`，这里保存旧 `%rbx`，本次值为 `0x7fffffffe178`。再执行 `movq %rdi,%rbx`，将保存区起点 `0x7fffffffe080` 交给 `%rbx`。

![暂存上级调用者可能使用的rdi值参数_1.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E4%B8%8A%E7%BA%A7%E8%B0%83%E7%94%A8%E8%80%85%E5%8F%AF%E8%83%BD%E4%BD%BF%E7%94%A8%E7%9A%84rdi%E5%80%BC%E5%8F%82%E6%95%B0_1.png)

![暂存上级调用者可能使用的rdi值参数_2.png](https://img.mdwind.online/img/%E6%9A%82%E5%AD%98%E4%B8%8A%E7%BA%A7%E8%B0%83%E7%94%A8%E8%80%85%E5%8F%AF%E8%83%BD%E4%BD%BF%E7%94%A8%E7%9A%84rdi%E5%80%BC%E5%8F%82%E6%95%B0_2.png)

于是，后面通过 `%rbx` 就能访问保存区。函数结束前，再从栈上恢复进入本函数时的 `%rbx`。

最后执行 `subq $0x8,%rsp`，将 `%rsp` 调整到 `0x7fffffffe040`。

![栈对齐_1.png](https://img.mdwind.online/img/%E6%A0%88%E5%AF%B9%E9%BD%90_1.png)

![栈对齐_2.png](https://img.mdwind.online/img/%E6%A0%88%E5%AF%B9%E9%BD%90_2.png)

本层共为两个寄存器的保存及额外空间使用了 24 字节。当前 `%rsp` 为 16 字节对齐，供后续可能发生的调用使用；本次因 `savemask=0`，不会走到实际调用信号阻塞集保存接口的分支。

| 本层位置 | 本次地址 | 内容 |
| --- | --- | --- |
| `%rbp+0x08` | `0x7fffffffe058` | 最初保存调用的返回地址 |
| `%rbp` | `0x7fffffffe050` | 旧 `%rbp=0x7fffffffe0f0` |
| `%rbp-0x08` | `0x7fffffffe048` | 旧 `%rbx=0x7fffffffe178` |
| `%rbp-0x10` | `0x7fffffffe040` | 额外预留空间，当前 `%rsp` 所在位置 |

## 不保存阻塞集，写入对应标志

```asm
0x7ffff7c44f60 <+16>: testl %esi, %esi
0x7ffff7c44f62 <+18>: jne   0x7ffff7c44f70 ; <+32>
0x7ffff7c44f64 <+20>: movl  %esi, 0x40(%rbx)
```

![检查是否还要保存信号掩码的标记位_1.png](https://img.mdwind.online/img/%E6%A3%80%E6%9F%A5%E6%98%AF%E5%90%A6%E8%BF%98%E8%A6%81%E4%BF%9D%E5%AD%98%E4%BF%A1%E5%8F%B7%E6%8E%A9%E7%A0%81%E7%9A%84%E6%A0%87%E8%AE%B0%E4%BD%8D_1.png)

![检查是否还要保存信号掩码的标记位_2.png](https://img.mdwind.online/img/%E6%A3%80%E6%9F%A5%E6%98%AF%E5%90%A6%E8%BF%98%E8%A6%81%E4%BF%9D%E5%AD%98%E4%BF%A1%E5%8F%B7%E6%8E%A9%E7%A0%81%E7%9A%84%E6%A0%87%E8%AE%B0%E4%BD%8D_2.png)

`%esi` 是前面由 `_setjmp` 清零的第二个实参，表示本次不保存信号阻塞集。`testl %esi,%esi` 的结果为零，ZF 置一。

![不保存继续运行_1.png](https://img.mdwind.online/img/%E4%B8%8D%E4%BF%9D%E5%AD%98%E7%BB%A7%E7%BB%AD%E8%BF%90%E8%A1%8C_1.png)

![不保存继续运行_2.png](https://img.mdwind.online/img/%E4%B8%8D%E4%BF%9D%E5%AD%98%E7%BB%A7%E7%BB%AD%E8%BF%90%E8%A1%8C_2.png)

`jne` 在 ZF 为零时才跳转，本次继续执行 `<+20>`，把 `%esi` 中的零写入 `%rbx+0x40`。

![将是否保存信号掩码的表示参数写回节点_1.png](https://img.mdwind.online/img/%E5%B0%86%E6%98%AF%E5%90%A6%E4%BF%9D%E5%AD%98%E4%BF%A1%E5%8F%B7%E6%8E%A9%E7%A0%81%E7%9A%84%E8%A1%A8%E7%A4%BA%E5%8F%82%E6%95%B0%E5%86%99%E5%9B%9E%E8%8A%82%E7%82%B9_1.png)

![将是否保存信号掩码的表示参数写回节点_2.png](https://img.mdwind.online/img/%E5%B0%86%E6%98%AF%E5%90%A6%E4%BF%9D%E5%AD%98%E4%BF%A1%E5%8F%B7%E6%8E%A9%E7%A0%81%E7%9A%84%E8%A1%A8%E7%A4%BA%E5%8F%82%E6%95%B0%E5%86%99%E5%9B%9E%E8%8A%82%E7%82%B9_2.png)

按结构体对照，这就是：

```c
state->mask_was_saved = 0; // 本次没有保存信号阻塞集
```

目标地址为 `0x7fffffffe0c0`，写入宽度为四字节。它记录的是保存行为；线程当前的信号阻塞集在这里没有被清空或恢复。

## 准备返回值，恢复本层使用的寄存器

```asm
0x7ffff7c44f67 <+23>: xorl %eax, %eax
0x7ffff7c44f69 <+25>: movq -0x8(%rbp), %rbx
0x7ffff7c44f6d <+29>: leave
0x7ffff7c44f6e <+30>: retq
```

![准备 setjmp 返回值_1.png](https://img.mdwind.online/img/%E5%87%86%E5%A4%87%20setjmp%20%E8%BF%94%E5%9B%9E%E5%80%BC_1.png)

![准备 setjmp 返回值_2.png](https://img.mdwind.online/img/%E5%87%86%E5%A4%87%20setjmp%20%E8%BF%94%E5%9B%9E%E5%80%BC_2.png)

`xorl %eax,%eax` 将 `%eax` 清零，提供这次保存调用正常返回时的返回值。之后，调用者将使用这个零选择正常执行路径。

![恢复  rbx_1.png](https://img.mdwind.online/img/%E6%81%A2%E5%A4%8D%20%20rbx_1.png)

![恢复  rbx_2.png](https://img.mdwind.online/img/%E6%81%A2%E5%A4%8D%20%20rbx_2.png)

`movq -0x8(%rbp),%rbx` 从 `0x7fffffffe048` 读回旧值，恢复 `%rbx=0x7fffffffe178`。本层对 `%rbx` 的临时使用到这里结束。

接着执行 `leave`。

![将两个栈指针恢复为退出这一层的状态_1.png](https://img.mdwind.online/img/%E5%B0%86%E4%B8%A4%E4%B8%AA%E6%A0%88%E6%8C%87%E9%92%88%E6%81%A2%E5%A4%8D%E4%B8%BA%E9%80%80%E5%87%BA%E8%BF%99%E4%B8%80%E5%B1%82%E7%9A%84%E7%8A%B6%E6%80%81_1.png)

![将两个栈指针恢复为退出这一层的状态_2.png](https://img.mdwind.online/img/%E5%B0%86%E4%B8%A4%E4%B8%AA%E6%A0%88%E6%8C%87%E9%92%88%E6%81%A2%E5%A4%8D%E4%B8%BA%E9%80%80%E5%87%BA%E8%BF%99%E4%B8%80%E5%B1%82%E7%9A%84%E7%8A%B6%E6%80%81_2.png)

它可以按下面两个动作理解：

```asm
movq %rbp, %rsp
popq %rbp
```

先将 `%rsp` 移到本层栈帧基准 `0x7fffffffe050`，撤去基准以下的使用空间；再从该位置取出旧 `%rbp`，使 `%rbp` 恢复为 `0x7fffffffe0f0`。弹栈后，`%rsp` 到达 `0x7fffffffe058`，重新指向最初保存调用的返回地址。

![再退出一次 恢复回到 main 调用者状态_1.png](https://img.mdwind.online/img/%E5%86%8D%E9%80%80%E5%87%BA%E4%B8%80%E6%AC%A1%20%E6%81%A2%E5%A4%8D%E5%9B%9E%E5%88%B0%20main%20%E8%B0%83%E7%94%A8%E8%80%85%E7%8A%B6%E6%80%81_1.png)

![再退出一次 恢复回到 main 调用者状态_2.png](https://img.mdwind.online/img/%E5%86%8D%E9%80%80%E5%87%BA%E4%B8%80%E6%AC%A1%20%E6%81%A2%E5%A4%8D%E5%9B%9E%E5%88%B0%20main%20%E8%B0%83%E7%94%A8%E8%80%85%E7%8A%B6%E6%80%81_2.png)

最后的 `retq` 取出 `0x7ffff7c2a181` 并转移执行流，`%rsp` 随之增加八字节，回到 `0x7fffffffe060`。这与上一段保存的接续栈位置相同。

这里经历的是第一次调用的正常返回。虽然实现中经过了 `_setjmp`、`__sigsetjmp` 和后续保存函数，但中间用 `jmp` 衔接，最终只有最初那条 `callq` 对应的返回地址需要由 `retq` 使用。

返回后的关键状态为：

```text
rip = 0x7ffff7c2a181    运行库调用者 <+49>
rsp = 0x7fffffffe060   保存调用返回后的栈位置
rbp = 0x7fffffffe0f0   运行库调用者的栈帧基准
eax = 0               首次正常返回值
```

运行库随后检查 `%eax`：

```asm
0x7ffff7c2a181 <+49>: endbr64
0x7ffff7c2a185 <+53>: testl %eax, %eax
0x7ffff7c2a187 <+55>: jne   0x7ffff7c2a1d1 ; <+129>
```

本次为零，继续向下填写节点关联信息并登记当前节点。若以后经相应的非局部恢复路径重新接续到这里，非零返回值则会使执行流进入另一条处理分支。节点登记的具体指令留到下一段继续对照。

---

### 材料与实现依据

本段插图沿用静态图第 33～48 步，共十六组、三十二张，按执行顺序排列。每组两张展示同一状态下不同范围的栈空间，箭头指向当时待执行的指令。影子栈恢复端及线程取消路径的区别依据源码说明，本次图中记录的是跳过 SSP 保存、随后正常返回的过程。

- [glibc：通用上下文恢复中的影子栈调整](https://codebrowser.dev/glibc/glibc/sysdeps/x86_64/__longjmp.S.html)
- [glibc：取消专用恢复路径](https://codebrowser.dev/glibc/glibc/sysdeps/x86/longjmp.c.html)
- [glibc：线程启动及私有区复用说明](https://codebrowser.dev/glibc/glibc/nptl/pthread_create.c.html)
- [glibc：保存逻辑](https://github.com/bminor/glibc/blob/master/sysdeps/x86_64/setjmp.S)
- [glibc：信号阻塞集保存标志与正常返回](https://codebrowser.dev/glibc/glibc/setjmp/sigjmp.c.html)
