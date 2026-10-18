# 附录 A · 第五段：关联清理边界，登记恢复节点

保存函数已经正常返回，运行库重新取得执行权。此时，局部对象中已经记录了接续所需的机器状态，接下来还要把它与线程原有的恢复关系关联起来，并登记为当前恢复节点。

这段代码先检查保存调用的返回值，再填写两个指针成员，最后更新线程中的节点入口。

```asm
0x7ffff7c2a181 <+49>: endbr64
0x7ffff7c2a185 <+53>: testl  %eax, %eax
0x7ffff7c2a187 <+55>: jne    0x7ffff7c2a1d1 ; <+129>

0x7ffff7c2a189 <+57>: movq   %fs:0x300, %rax
0x7ffff7c2a192 <+66>: movq   %rax, -0x28(%rbp)
0x7ffff7c2a196 <+70>: movq   %fs:0x2f8, %rax
0x7ffff7c2a19f <+79>: movq   %rax, -0x20(%rbp)
0x7ffff7c2a1a3 <+83>: leaq   -0x70(%rbp), %rax
0x7ffff7c2a1a7 <+87>: movq   %rax, %fs:0x300
```

## 这里所说的清理，具体清理什么

线程执行过程中，可能已经持有互斥锁、申请了堆内存，或者打开了文件。正常完成工作时，可以在后续代码中解锁、释放内存、关闭文件。如果线程提前退出或被取消，这些后续操作就可能没有机会执行。

线程清理处理器（cleanup handler）为这种情况提供了一种安排：程序事先登记一个回调函数及其参数，在线程取消或通过 `pthread_exit` 退出时，由线程库按相应规则调用它。回调可以执行解锁、释放某块内存等收尾操作，具体负责哪些资源，由登记的函数决定。

这里涉及的 `%fs:0x2f8`，对应本次 glibc 实现中旧式清理处理器链表的头指针。链上的节点记录清理回调、参数以及前一个节点等信息。“旧式”指 glibc 为兼容维护的内部表示；线程清理接口的实现还可以与栈展开机制配合。

登记顺序具有明确作用：对相应清理处理器，按后登记先处理的方式组织，适合与嵌套的资源取得和释放相对应。完整的线程退出还包含其他阶段，例如线程专有数据析构；这些阶段之间的关系，需要结合整个退出机制来看。一条清理链表的顺序，描述的是它所管理的那组处理器。

本段没有登记新的旧式清理处理器。它做的是保存当前清理链表的位置，使新的恢复节点带有与之对应的清理边界。

## 首次返回，继续执行登记路径

```asm
0x7ffff7c2a181 <+49>: endbr64
0x7ffff7c2a185 <+53>: testl %eax, %eax
0x7ffff7c2a187 <+55>: jne   0x7ffff7c2a1d1 ; <+129>
```

![可落点标记_7.png](https://img.mdwind.online/img/%E5%8F%AF%E8%90%BD%E7%82%B9%E6%A0%87%E8%AE%B0_7.png)

![可落点标记_8.png](https://img.mdwind.online/img/%E5%8F%AF%E8%90%BD%E7%82%B9%E6%A0%87%E8%AE%B0_8.png)

上一段的 `retq` 已将执行流送回 `<+49>`，当前 `%rsp=0x7fffffffe060`，`%rbp=0x7fffffffe0f0`，`%eax=0`。

经过入口标记后，执行 `testl %eax,%eax`，检查保存调用的返回值。

![检查返回值_1.png](https://img.mdwind.online/img/%E6%A3%80%E6%9F%A5%E8%BF%94%E5%9B%9E%E5%80%BC_1.png)

![检查返回值_2.png](https://img.mdwind.online/img/%E6%A3%80%E6%9F%A5%E8%BF%94%E5%9B%9E%E5%80%BC_2.png)

零与自身按位与仍为零，ZF 置一。后面的 `jne` 在结果非零时跳转，本次继续向下。

![不跳过继续_1.png](https://img.mdwind.online/img/%E4%B8%8D%E8%B7%B3%E8%BF%87%E7%BB%A7%E7%BB%AD_1.png)

![不跳过继续_2.png](https://img.mdwind.online/img/%E4%B8%8D%E8%B7%B3%E8%BF%87%E7%BB%A7%E7%BB%AD_2.png)

这样，同一个接续位置就能区分两种执行情况：首次保存完成后正常返回，继续填写和登记节点；以后经相应的恢复路径接续到这里，非零返回值则引导执行流进入 `<+129>` 的退出处理分支。

## 保存此前登记的恢复节点

```asm
0x7ffff7c2a189 <+57>: movq %fs:0x300, %rax
0x7ffff7c2a192 <+66>: movq %rax, -0x28(%rbp)
```

本次 glibc 的线程管理结构中，`cleanup_jmp_buf` 保存当前登记的恢复节点指针，机器码通过 `%fs:0x300` 访问这个成员。这里先将它读入 `%rax`。

![获取上次上下文恢复节点指针_1.png](https://img.mdwind.online/img/%E8%8E%B7%E5%8F%96%E4%B8%8A%E6%AC%A1%E4%B8%8A%E4%B8%8B%E6%96%87%E6%81%A2%E5%A4%8D%E8%8A%82%E7%82%B9%E6%8C%87%E9%92%88_1.png)

![获取上次上下文恢复节点指针_2.png](https://img.mdwind.online/img/%E8%8E%B7%E5%8F%96%E4%B8%8A%E6%AC%A1%E4%B8%8A%E4%B8%8B%E6%96%87%E6%81%A2%E5%A4%8D%E8%8A%82%E7%82%B9%E6%8C%87%E9%92%88_2.png)

本次图中读到的是零，表示当时这个入口没有指向此前的恢复节点。接下来，将这个值写入当前对象。

![将其写到当前上下文节点指针域_1.png](https://img.mdwind.online/img/%E5%B0%86%E5%85%B6%E5%86%99%E5%88%B0%E5%BD%93%E5%89%8D%E4%B8%8A%E4%B8%8B%E6%96%87%E8%8A%82%E7%82%B9%E6%8C%87%E9%92%88%E5%9F%9F_1.png)

![将其写到当前上下文节点指针域_2.png](https://img.mdwind.online/img/%E5%B0%86%E5%85%B6%E5%86%99%E5%88%B0%E5%BD%93%E5%89%8D%E4%B8%8A%E4%B8%8B%E6%96%87%E8%8A%82%E7%82%B9%E6%8C%87%E9%92%88%E5%9F%9F_2.png)

当前对象的起点是 `%rbp-0x70=0x7fffffffe080`。目标位置 `%rbp-0x28=0x7fffffffe0c8`，相对于对象起点的偏移正好为 `+0x48`，对应展开结构体中的 `prev`。

```c
current_node->prev = thread->cleanup_jmp_buf;
```

本次写入空指针；在已有节点的情况下，同样的代码会保存旧节点地址。以后沿当前节点的 `prev`，就能继续找到此前登记的恢复关系。

## 记录登记时的清理链表位置

```asm
0x7ffff7c2a196 <+70>: movq %fs:0x2f8, %rax
0x7ffff7c2a19f <+79>: movq %rax, -0x20(%rbp)
```

`<+70>` 读取线程旧式清理链表的当前头指针，本次读到的值也是零。

![获取上次上下文恢复清理方法的表头_1.png](https://img.mdwind.online/img/%E8%8E%B7%E5%8F%96%E4%B8%8A%E6%AC%A1%E4%B8%8A%E4%B8%8B%E6%96%87%E6%81%A2%E5%A4%8D%E6%B8%85%E7%90%86%E6%96%B9%E6%B3%95%E7%9A%84%E8%A1%A8%E5%A4%B4_1.png)

![获取上次上下文恢复清理方法的表头_2.png](https://img.mdwind.online/img/%E8%8E%B7%E5%8F%96%E4%B8%8A%E6%AC%A1%E4%B8%8A%E4%B8%8B%E6%96%87%E6%81%A2%E5%A4%8D%E6%B8%85%E7%90%86%E6%96%B9%E6%B3%95%E7%9A%84%E8%A1%A8%E5%A4%B4_2.png)

随后，把这个指针写入 `%rbp-0x20`。

![依旧使用之前的方法集_1.png](https://img.mdwind.online/img/%E4%BE%9D%E6%97%A7%E4%BD%BF%E7%94%A8%E4%B9%8B%E5%89%8D%E7%9A%84%E6%96%B9%E6%B3%95%E9%9B%86_1.png)

![依旧使用之前的方法集_2.png](https://img.mdwind.online/img/%E4%BE%9D%E6%97%A7%E4%BD%BF%E7%94%A8%E4%B9%8B%E5%89%8D%E7%9A%84%E6%96%B9%E6%B3%95%E9%9B%86_2.png)

该地址为 `0x7fffffffe0d0`，相对于对象起点偏移 `+0x50`，对应 `saved_cleanup_head`。

```c
current_node->saved_cleanup_head = thread->cleanup;
```

这个成员记录的是建立当前恢复关系时，线程旧式清理处理器链表的头指针，也就是当前恢复节点对应的清理边界。它不新增清理处理器，只保存登记这一刻的清理链位置。此后如果又压入了新的清理处理器，那么在线程取消、`pthread_exit` 或内部展开回到当前恢复节点时，线程退出机制会从当前清理链头开始，按后进先出的顺序执行登记在该边界之后的处理器，直到到达 `saved_cleanup_head` 保存的边界为止；边界及更早的处理器留给外层恢复关系。控制流最后由 `longjmp` 送回当前节点的 `setjmp` 点，`setjmp` 返回非零，进入该节点的退出处理分支。清理处理器的执行和 `longjmp` 的控制转移是两件事：前者由线程退出/展开逻辑按边界驱动，后者只负责把执行流送到恢复点。

例如，登记恢复节点时已有清理处理器甲，节点保存甲的位置；后来又依次登记乙、丙。展开到这一层时，先执行丙、乙，遇到甲这个保存的边界后停止；甲留给外层关系处理。这里“边界之外”是相对于当前恢复节点而言：从内层看是后压入的乙、丙，从外层看甲才是它要处理的范围。实际展开还会结合当前栈帧位置，决定这一轮应推进到哪里。本次保存的边界为空，表示登记这一刻旧式清理链上没有处理器；保存空边界与非空边界使用的是同一套指令。

这里要区分两条链：`cleanup_jmp_buf` 链通过 `prev` 连接恢复/展开节点，当前对象是这条链上的新节点；旧式清理处理器链通过 `%fs:0x2f8` 维护，当前代码只保存它的边界，没有向它压入新节点。因此，`saved_cleanup_head` 记录的是“当前恢复关系对应的清理边界”，不是“当前对象在清理链上的位置”。本段只更新 `%fs:0x300`，不写 `%fs:0x2f8`，正说明这里登记的是恢复关系，而不是新的清理处理器。

恢复节点链与清理处理器链在这里发生关联：`prev` 联系此前的恢复节点，`saved_cleanup_head` 记录当前恢复节点对应的旧式清理边界。二者分别回答“恢复到哪一层”和“这一层清理到哪里为止”。

## 将当前对象登记为新的恢复节点

```asm
0x7ffff7c2a1a3 <+83>: leaq -0x70(%rbp), %rax
0x7ffff7c2a1a7 <+87>: movq %rax, %fs:0x300
```

前两个指针成员写好以后，`leaq` 再次计算当前对象的地址。

![读取当前上下文恢复节点的基地址_1.png](https://img.mdwind.online/img/%E8%AF%BB%E5%8F%96%E5%BD%93%E5%89%8D%E4%B8%8A%E4%B8%8B%E6%96%87%E6%81%A2%E5%A4%8D%E8%8A%82%E7%82%B9%E7%9A%84%E5%9F%BA%E5%9C%B0%E5%9D%80_1.png)

![读取当前上下文恢复节点的基地址_2.png](https://img.mdwind.online/img/%E8%AF%BB%E5%8F%96%E5%BD%93%E5%89%8D%E4%B8%8A%E4%B8%8B%E6%96%87%E6%81%A2%E5%A4%8D%E8%8A%82%E7%82%B9%E7%9A%84%E5%9F%BA%E5%9C%B0%E5%9D%80_2.png)

得到 `%rax=0x7fffffffe080`。这次计算地址后，不再将它传给保存函数，而是准备交给线程的当前恢复节点入口。

![将线程维护结构体相关成员索引更新_1.png](https://img.mdwind.online/img/%E5%B0%86%E7%BA%BF%E7%A8%8B%E7%BB%B4%E6%8A%A4%E7%BB%93%E6%9E%84%E4%BD%93%E7%9B%B8%E5%85%B3%E6%88%90%E5%91%98%E7%B4%A2%E5%BC%95%E6%9B%B4%E6%96%B0_1.png)

![将线程维护结构体相关成员索引更新_2.png](https://img.mdwind.online/img/%E5%B0%86%E7%BA%BF%E7%A8%8B%E7%BB%B4%E6%8A%A4%E7%BB%93%E6%9E%84%E4%BD%93%E7%9B%B8%E5%85%B3%E6%88%90%E5%91%98%E7%B4%A2%E5%BC%95%E6%9B%B4%E6%96%B0_2.png)

执行 `<+87>` 后，线程的 `cleanup_jmp_buf` 指向当前栈上的对象。前面保存的机器状态及刚填好的关联信息，可以通过这个线程成员找到。

把主要数据流放在一起，就是下面的 C 风格表示：

```c
/* current_node 指向本次栈上的恢复节点。 */
current_node->prev = thread->cleanup_jmp_buf;
current_node->saved_cleanup_head = thread->cleanup;

/* 完成关联信息填写后，更新当前恢复节点入口。 */
thread->cleanup_jmp_buf = current_node;
```

以线程保存的指针为链表入口来看，这是头插：新节点成为当前入口，旧入口保存在新节点的 `prev` 中。本次旧入口为空，所以形成的关系是线程入口指向当前节点，当前节点的 `prev` 为空。

这里登记了新的恢复关系，没有同时登记新的旧式清理处理器。清理入口继续沿用，只把它的当前位置保存到当前节点，代码中也就没有向 `%fs:0x2f8` 写回的操作。

## 对照本段填写的成员

| 位置 | 本次地址或访问形式 | 含义 | 本次操作后的值 |
| --- | --- | --- | --- |
| 节点 `+0x48` | `0x7fffffffe0c8` | `prev`，此前恢复节点 | `0` |
| 节点 `+0x50` | `0x7fffffffe0d0` | `saved_cleanup_head`，登记时的清理边界 | `0` |
| 线程当前恢复入口 | `%fs:0x300` | `cleanup_jmp_buf` | `0x7fffffffe080` |
| 线程旧式清理入口 | `%fs:0x2f8` | `cleanup` | 保持原值，本次为 `0` |

前几段先提供空间、填写机器状态，这一段则完成关联和登记。当前对象仍位于运行库函数的栈帧中；线程中的入口保存它的地址，将它纳入运行库维护的恢复关系。接下来，执行流继续准备 `main` 的实参，并调用 `main`。

---

### 材料与实现依据

本段插图按原静态图第 49～57 步排列，共九组、十八张。箭头指向待执行的指令，每组两张展示同一状态下不同范围的栈空间。线程字段偏移按本次机器码使用的布局说明，成员命名和管理含义结合 glibc 源码核对。

- [glibc：main 调用外围的节点关联与登记](https://codebrowser.dev/glibc/glibc/sysdeps/nptl/libc_start_call_main.h.html)
- [glibc：pthread_unwind_buf 中的 prev 与 cleanup 成员](https://codebrowser.dev/glibc/glibc/nptl/descr.h.html)
- [glibc：展开过程中处理旧式清理链及保存边界](https://codebrowser.dev/glibc/glibc/nptl/unwind.c.html)
