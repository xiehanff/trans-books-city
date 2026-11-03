# 附录 A　用 GDB 或 LLDB 调试汇编代码

*Appendix A: Debugging Assembly Code with GDB or LLDB*

总有一天，你的编译器会生成行为不正确的汇编代码，而你需要弄清楚原因。遇到这种情况时，命令行调试器对于理解究竟哪里出了问题来说是不可或缺的。调试器可以让你暂停正在运行的程序、一次一条指令地单步执行，并在不同的位置检查程序状态。你可以使用 GDB（GNU 调试器）或 LLDB（LLVM 项目的调试器）来调试你的编译器生成的汇编代码。如果你使用 Linux，我推荐 GDB；如果你使用 macOS，我推荐 LLDB（我认为 GDB 处理汇编的界面要稍微好用一些，但要让它在 macOS 上跑起来可能颇费周折）。

本附录是用 GDB 或 LLDB 调试汇编程序的简明指南。如果你从未用过调试器，这里会介绍你需要掌握的基础知识；本附录还专门涵盖了处理汇编代码时最需要用到的那些命令和选项——即使你已经习惯用这些工具来调试源代码，这部分内容对你来说可能也是新的。我为两款调试器分别准备了独立的操作演练；尽管它们的功能非常相似，但许多命令的细节并不相同。请选择与你所用的调试器对应的演练跟着操作。

在开始之前，你应当先熟悉第 1 章和第 2 章介绍的汇编基础知识。后文还会涉及后续章节中的一些汇编内容，如果你还没有读到那里，暂时跳过也没关系。

## 示例程序

我们将使用清单 A-1 中的汇编程序作为贯穿本附录的示例。

```asm
    .data
    .align 4
❶ integer:
    .long 100
    .align 8
❷ dbl:
    .double 3.5
    .text
    .globl main
❸ main:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $16, %rsp
    # 调用一个函数
 ❹ callq   f
 ❺ # 往寄存器里放一些数据
    movl    $0x87654321, %eax
    movsd   dbl(%rip), %xmm0
    # 往栈上放一些数据
    movl    $0xdeadbeef, -4(%rbp)
    movl    $0, -8(%rbp)
    movl    $-1, -12(%rbp)
    movl    $0xfeedface, -16(%rbp)
 ❻ # 初始化循环计数器
    movl    $25, %ecx
.L_loop_start:
    # 递减计数器
    subl    $1, %ecx
    cmpl    $0, %ecx
    # 跳回循环开头
    jne     .L_loop_start
    # 返回 0
    movl    $0, %eax
    movq    %rbp, %rsp
    popq    %rbp
    ret
    .text
    .globl  f
f:
    movl    $1, %eax
    ret
    .section .note.GNU-stack,"",@progbits
```

*清单 A-1：一个毫无实际用途的汇编程序*

这个程序做不成任何有用的事；它只是给我们机会去试用调试器最重要的功能。它包含了几个供我们检查的静态变量：integer ❶ 和 dbl ❷。在 main ❸ 中，它首先调用一个非常小的函数 f，让我们练习单步进入和跳出函数调用 ❹，然后把一些数据移入寄存器和栈，让我们练习检查程序状态 ❺。它最后以一个循环收尾，该循环每轮迭代都将 ECX 递减，减到 0 时停止 ❻。我们将用这个循环来练习设置条件断点。

从 https://norasandler.com/book/#appendix-a 下载这个程序，然后把它保存为 hello_debugger.s。这个文件有 Linux 和 macOS 两个不同的版本，请务必选择与你的操作系统匹配的那个。

保存好文件之后，对它进行汇编和链接，并确认它能运行：

```bash
$ gcc hello_debugger.s -o hello_debugger
$ ./hello_debugger
```

在 macOS 上，汇编和链接该文件时要加上 `-g` 选项：

```bash
$ gcc -g hello_debugger.s -o hello_debugger
```

`-g` 选项会生成额外的调试信息。以后在汇编、链接你自己编译器输出的汇编代码进行调试时，也请记得加上它。

现在你可以开始演练了。如果你使用 GDB，请跟随下一节的演练；如果你使用 LLDB，请直接跳到第 687 页的“用 LLDB 调试”。

## 用 GDB 调试

运行下面这条命令启动 GDB：

```text
$ gdb hello_debugger
--snip--
(gdb)
```

这会把 hello_debugger 设为要调试的可执行文件，但并不会真正执行它。在开始运行这个可执行文件之前，我们先配置一下界面，让处理汇编代码更轻松。

### 配置 GDB 界面

在 GDB 会话期间，你可以打开多个不同的文本窗口，显示关于被调试程序的不同信息。就我们的目的而言，其中最重要的是汇编（assembly）窗口，它在我们单步执行时显示汇编代码。寄存器（register）窗口也很有用；默认情况下，它会显示每个通用寄存器的当前内容。

`layout` 命令控制哪些窗口可见。我们来打开汇编窗口和寄存器窗口：

```text
(gdb) layout asm
(gdb) layout reg
```

现在你的终端里应该能看到三个窗口：寄存器窗口、汇编窗口，以及带有 (gdb) 提示符的命令窗口。看起来应该和图 A-1 差不多。

*图 A-1：打开了汇编窗口和寄存器窗口的 GDB 会话*

在启动程序之前，寄存器窗口不会显示任何信息。

你可以在当前获得焦点的窗口里滚动。使用 `focus` 命令可以切换获得焦点的窗口：

```text
(gdb) focus cmd
(gdb) focus asm
(gdb) focus regs
```

### 启动和停止程序

接下来，我们要设置一个断点（breakpoint）——即调试器将暂停程序的位置——然后让程序一直运行到这个断点。如果不先设置断点就直接启动程序，程序会一口气运行到结束而不暂停，这样并没有多大用处。

命令 `break <function name>` 会在函数起始处设置断点。我们在 main 的入口设置一个断点：

```text
(gdb) break main
Breakpoint 1 at 0x112d
```

现在启动程序：

```text
(gdb) run
Starting program: /home/ubuntu/hello_debugger

❶ Breakpoint 1, 0x000055555555512d in main ()
```

这条命令的输出告诉我们，程序命中了我们刚刚设置的断点 ❶。注意，汇编窗口中当前指令被高亮显示，寄存器窗口中则显示了各通用寄存器的当前值，如图 A-2 所示。

*图 A-2：程序停在断点处时的 GDB 会话*

程序暂停之后，有几条命令可以让它继续向前执行：

`continue` 恢复程序运行，直到命中另一个断点或程序退出。

`finish` 恢复程序运行，并在从当前函数返回时再次暂停。

`stepi` 执行下一条指令，然后暂停。它会单步进入 call 指令，在被调用函数的第一条指令处暂停。命令 `stepi <n>` 会执行 n 条指令。

`nexti` 执行下一条指令，然后暂停。它会单步越过 call 指令，在当前函数中 call 之后的下一条指令处暂停。命令 `nexti <n>` 会执行 n 条指令。

大多数 GDB 命令都可以缩写成一两个字母：可以用 `c` 代替 `continue`、`b` 代替 `break`、`si` 代替 `stepi`，等等。表 A-1（第 687 页）给出了我们讨论过的所有命令的完整形式和缩写形式。

> **警告**
>
> `nexti` 和 `stepi` 命令单步执行的是汇编指令，而 `next` 和 `step` 命令单步执行的是原始源文件中的代码行。由于我们完全没有原始源文件的信息，输入这两条命令中的任何一条都会让程序一直运行到当前函数结束。这两条命令分别缩写为 `n` 和 `s`，因此当你本想用 `nexti` 或 `stepi` 时，很容易失手敲成它们。

我们来试试这些新命令。首先执行两条指令，这应该会让我们进入对 f 的调用：

```text
(gdb) stepi 2
0x0000555555555176 in ❶ f ()
```

从命令输出 ❶ 和汇编窗口中被高亮的指令可以看出，我们现在停在 f 而不是 main 里。接下来，我们从 f 返回：

```text
(gdb) finish
Run till exit from #0  0x0000555555555176 in f ()
0x0000555555555136 in main ()
```

现在我们回到了 main 中，位于 callq 紧后面的那条指令上。继续执行：

```text
(gdb) continue
Continuing.
[Inferior 1 (process 82326) exited normally]
```

由于没有再命中任何断点，程序一直运行到退出。要继续调试，我们就得重新启动它：

```text
(gdb) run
Starting program: /home/ubuntu/hello_debugger

Breakpoint 1, 0x000055555555512d in main ()
```

现在我们又暂停在 main 的开头。我们再次向前执行两条指令，不过这次用 `nexti` 越过 f，而不是进入它：

```text
(gdb) nexti 2
0x0000555555555136 in main ()
```

我们再次停在了 callq 紧后面的那条指令上。

#### 按地址设置断点

除了在函数上设置断点，还可以在具体的机器指令上设置断点。我们在指令 movl 0xdeadbeef, -4(%rbp) 上设一个断点。首先，在汇编窗口中找到这条指令，它看起来应该像这样：

```text
    ❶ 0x555555555143 ❷ <main+26>        movl    0xdeadbeef, -4(%rbp)
```

指令在内存中的地址位于这一行的开头 ❶，其后是该地址相对函数起始处的字节偏移量 ❷。确切的地址在你的机器上可能不同，但偏移量应该一致。要设置这个断点，可以输入

```text
(gdb) break *main+26
```

或者

```text
(gdb) break *MEMORY_ADDRESS
```

其中 MEMORY_ADDRESS 是你在汇编窗口中找到的地址。`*` 符号告诉 GDB，我们指定的是一个确切地址，而不是函数名。

#### 管理断点

我们来列出设置过的所有断点：

```text
(gdb) info break
Num     Type           Disp Enb Address            What
1       breakpoint     keep y   0x000055555555512d <main+4>
        breakpoint already hit 1 time
2       breakpoint     keep y   0x0000555555555143 <main+26>
```

每个断点都有一个唯一的编号，需要删除、禁用或修改断点时可以用它来指代。我们来删除断点 1：

```text
(gdb) delete 1
```

接下来，我们来看几种检查程序状态的不同方法。

### 打印表达式

你可以用命令 `print/<format> <expr>` 打印表达式，其中：

`<format>` 是单字母的格式说明符。printf 里用到的大多数格式说明符这里都能用：x 表示把值显示为十六进制整数，d 表示显示为有符号整数，等等。

`<expr>` 是任意表达式。表达式既可以引用正在运行的程序中的寄存器、内存地址和符号，也可以包含 C 的运算，比如算术运算、指针解引用和强制类型转换。

我们来看几个例子。现在程序应该停在指令 movl 0x87654321, %eax 上。我们先单步执行这条指令，然后用几种不同的格式打印 EAX 的值：

```text
(gdb) stepi
--snip--
(gdb) print $eax
$1 = ❶ -2023406815
(gdb) print/x $eax
$2 = ❷ 0x87654321
(gdb) print/u $eax
$3 = ❸ 2271560481
```

默认情况下，GDB 把通用寄存器中的值格式化为有符号整数 ❶。这里我们又以十六进制 ❷ 和无符号整数 ❸ 的形式显示了 EAX 中的值。符号 $1、$2 等是便利变量（convenience variable），GDB 会自动生成它们来保存每个表达式的求值结果。

完整的格式说明符列表可以在 `x` 命令的文档里找到，我们马上就会详细讨论这个命令：

```text
(gdb) help x
--snip--
Format letters are o(octal), x(hex), d(decimal), u(unsigned decimal),
  t(binary), f(float), a(address), i(instruction), c(char), s(string)
  and z(hex, zero padded on the left).
--snip--
```

第 13 章介绍了保存浮点值的 XMM 寄存器。程序中的下一条指令 movsd dbl(%rip), %xmm0 会把静态变量 dbl 中的值 3.5 复制到 XMM0 中。

我们单步执行这条指令，然后检查 XMM0：

```text
(gdb) stepi
--snip--
(gdb) print $xmm0
$4 = {v4_float = {0, 2.1875, 0, 0}, v2_double = {3.5, 0}, v16_int8 = {0, 0, 0, 0,
0, 0, 12, 64, 0, 0, 0, 0, 0, 0, 0, 0}, v8_int16 = {0, 0, 0, 16396, 0, 0, 0, 0},
v4_int32 = {0, 1074528256, 0, 0}, v2_int64 = {4615063718147915776, 0}, uint128 =
4615063718147915776}
```

GDB 给我们展示了同一份数据的许多不同视图：v4_float 把寄存器内容显示为 4 个 32 位浮点数组成的数组，v2_double 把它显示为 64 位 double 组成的数组，等等。由于我们只用 XMM 寄存器保存单个 double，你始终可以用下面这样的命令来检查它们：

```text
(gdb) print $xmm0.v2_double[0]
$5 = 3.5
```

这会打印寄存器低 64 位的值，并把它解释为 double。

除了寄存器，我们还可以打印符号表中对象的值。我们来检查这个程序里的两个静态变量 integer 和 dbl：

```text
(gdb) print (long) integer
$6 = 100
(gdb) print (double) dbl
$7 = 3.5
```

由于 GDB 没有关于这些对象类型的任何信息，我们必须显式地把它们强制转换成正确的类型。

再来看几个更复杂表达式的例子。除了直接引用硬件寄存器这一点之外，这些表达式用的都是普通的 C 语法。

我们可以做基本的算术运算：

```text
(gdb) print/x $eax + 0x10
$8 = 0x87654331
```

我们可以调用当前程序或标准库中定义的函数。这里我们调用 f，它返回 1：

```text
(gdb) print (int) f()
$9 = 1
```

我们还可以解引用指针。先执行下一条指令 movl 0xdeadbeef, -4(%rbp)，然后检查 -4(%rbp) 处的值：

```text
(gdb) stepi
--snip--
(gdb) print/x *(int *)($rbp - 4)
$10 = 0xdeadbeef
```

首先，我们算出想要检查的内存地址 $rbp - 4；然后把这个地址强制转换成正确的指针类型 (int *)；最后用解引用运算符 * 对它解引用。这会得到一个整数，我们用 /x 说明符以十六进制把它打印出来。

接下来，我们来看一种更灵活的检查内存中数据的方法。

### 检查内存

我们可以用命令 `x/<n><format><unit> <expr>` 检查内存，其中：

`<n>` 是要显示的内存单元数（按 `<unit>` 指定的大小计）。

`<format>` 指定每个单元如何格式化。这些格式说明符与我们在 print 命令中使用的相同。

`<unit>` 是指定单元大小的单字母说明符：b 表示 1 字节（byte），h 表示 2 字节的半字（halfword），w 表示 4 字节的字（word），g 表示 8 字节的“巨字”（giant word）。

`<expr>` 是任意表达式，其求值结果必须是某个有效的内存地址。这类表达式与我们在 print 命令中使用的相同。

我们用 `x` 命令检查 -4(%rbp) 处的整数：

```text
(gdb) x/1xw ($rbp - 4)
❶ 0x7fffffffe2ac: ❷ 0xdeadbeef
```

这条命令让 GDB 以十六进制打印一个 4 字节的字。输出中既包含内存地址 ❶，也包含该地址处的值 ❷。

清单 A-1 中接下来的三条指令又往栈上存了三个整数：

```asm
    movl    $0, -8(%rbp)
    movl    $-1, -12(%rbp)
    movl    $0xfeedface, -16(%rbp)
```

我们用清单 A-2 中的命令单步执行这三条指令，然后打印出整个栈帧。

```text
(gdb) stepi 3
(gdb) x/6xw $rsp
0x7fffffffe2a0: ❶ 0xfeedface      0xffffffff     0x00000000     ❷ 0xdeadbeef
0x7fffffffe2b0: ❸ 0x00000000      0x00000000
```

*清单 A-2：向前单步执行三条指令，然后打印当前栈帧*

命令 `x/6xw $rsp` 让 GDB 从 RSP 中的地址开始打印六个 4 字节的字。之所以打印六个字，是因为这个函数的栈帧共 24 字节：在 main 开头，我们把 RBP 的旧值保存到了栈上，占 8 字节；随后又用指令 subq $16, %rsp 分配了另外 16 字节。请记住，RSP 始终保存着栈顶的地址，也就是最低的栈地址。

这条命令显示了保存到栈上的四个整数：0xfeedface 在最顶端 ❶，0xdeadbeef 在最底端 ❷，紧随其后的是 RBP 的旧值 ❸。在某些系统上，这个值是 0，因为我们已经处于最外层的栈帧；在另一些系统上，它会是一个有效的内存地址。

保存的 RBP 值位于当前栈帧的最底部。紧挨着它、落在调用者栈帧最顶端的位置，我们能找到调用者的返回地址——也就是我们从 main 返回时将要跳转到的地址。（第 9 章实现函数调用时我们详细讨论过这一点。）我们来检查这个返回地址：

```text
(gdb) x/4ag $rsp
0x7fffffffe2a0: 0xfffffffffeedface      0xdeadbeef00000000
0x7fffffffe2b0: 0x0     ❶ 0x7ffff7dee083 <__libc_start_main+243>
```

这条命令会打印四个 8 字节的“巨字”，从 RSP 中的地址处的值开始。a 说明符告诉 GDB 把这些值格式化成内存地址；也就是说，它会以十六进制打印每个地址，并且（如果可能的话）打印出该地址相对程序中最近符号的偏移量。由于函数名和静态变量名都定义在符号表中，GDB 能够显示汇编指令和静态数据的相对偏移量。至于栈地址、堆地址和无效地址，它则不会显示相对偏移量，因为那样做毫无意义。

输出的第一行是保存到栈上的那四个整数，现在显示为两个 8 字节的值，而不是四个 4 字节的值。下一行的空指针 0x0 是保存的 RBP 值。这三个 8 字节的值都不是有效地址，所以 GDB 无法显示它们相对符号的偏移量。栈上的下一个值就是返回地址 ❶。GDB 告诉我们，这是 __libc_start_main 中一条指令的地址——__libc_start_main 是标准库中负责调用 main 并在其退出后做清理的函数。

a 说明符让我们很容易认出返回地址和指向静态变量的指针。如果你的程序栈帧被破坏了，这一点尤其有用；找到每个栈帧的返回地址能帮你重新理清头绪。

### 设置条件断点

作为本次演练的收尾，我们来看看如何设置条件断点。只有当关联的条件为真时，程序才会在条件断点处暂停。条件可以是任意表达式；GDB 把求值结果为 0 的条件视为假，其他情况视为真。

我们在 hello_debugger 中最后一次循环迭代末尾的 jne 指令上设置一个断点。首先，需要在汇编窗口中找到这条指令。它应该位于函数起始处之后 65 字节的地方：

```text
    0x55555555516a <main+65>        jne    0x555555555164 <main+59>
```

我们设置一个条件断点，当 ECX 为 0 时在这条指令处暂停：

```text
(gdb) break *main+65 if $ecx == 0
```

由于这个循环会一直重复到 ECX 变为 0，条件 $ecx == 0 只会在最后一次迭代时为真。我们继续运行到这个断点，然后验证条件确实为真：

```text
(gdb) c
Continuing.

Breakpoint 3, 0x000055555555516a in main ()
(gdb) print $ecx
$11 = 0
```

到目前为止一切正常。如果你得到的 ECX 值不是这个，请检查断点是否设置正确：

```text
(gdb) info break
--snip--
3       breakpoint     keep y   0x000055555555516a ❶ <main+65>
        stop only if ❷ $ecx == 0
```

请确认你的断点位于 main+65 处 ❶，并且带有条件 $ecx == 0 ❷。如果你的断点不是这样，可能是哪里输错了；删掉它重新再试。

现在应该处于最后一次循环迭代，我们向前单步执行一条指令，确认跳转没有发生：

```text
(gdb) stepi
```

通常 jne 会跳回循环开头，但在最后一次迭代中，它会向前推进到下一条指令。

### 获取帮助

要了解这里没有讲到的命令和选项，请参阅 GDB 文档：https://sourceware.org/gdb/current/onlinedocs/gdb/index.html。如前文所见，你也可以在提示符下输入 `help` 来了解任何 GDB 命令的更多信息。例如，要查看 `run` 命令的文档，可以输入：

```text
(gdb) help run
Start debugged program.
You may specify arguments to give it.
--snip--
```

表 A-1 总结了我们介绍过的命令和选项，包括每个命令的完整形式和缩写形式（x 除外，它没法再缩写了）。两种形式接受的参数相同。

*表 A-1：GDB 命令总结*

| 命令 | 缩写 | 描述 |
| --- | --- | --- |
| run | r | 启动程序。 |
| continue | c | 恢复程序运行。 |
| finish | fin | 恢复程序运行并一直执行到当前函数退出。 |
| stepi [<n>] | si | 执行一条（或 n 条）指令，单步进入函数调用。 |
| nexti [<n>] | ni | 执行一条（或 n 条）指令，单步越过函数调用。 |
| break <loc> [if <cond>] | b | 在 <loc> 处设置断点（若提供了 <cond>，则以 <cond> 为条件）。 |
| info break | i b | 列出所有断点。（其他 info 子命令显示其他信息。） |
| delete [<id>] | d | 删除所有断点（或由 <id> 指定的断点）。 |
| print/<format> <expr> | p | 对 <expr> 求值，并按格式说明符 <format> 显示结果。 |
| x/<n><format><unit> <addr> | — | 从 <addr> 开始打印 n 个大小为 <unit> 的内存块，按 <format> 格式化。 |
| layout <window> | la | 打开 <window>。 |
| focus <window> | fs | 把焦点切换到 <window>。 |
| help <cmd> | h | 显示关于 <cmd> 的帮助文本。 |

现在，你已经可以用 GDB 开始调试了！

## 用 LLDB 调试

运行下面这条命令启动 LLDB：

```text
$ lldb hello_debugger
(lldb) target create "hello_debugger"
Current executable set to 'hello_debugger' (x86_64).
(lldb)
```

这会把 hello_debugger 设为要调试的可执行文件，但还不会执行它。如果出现提示，请输入你的用户名和密码，授予 LLDB 控制 hello_debugger 的权限。

### 启动和停止程序

接下来，我们要设置一个断点（breakpoint）——即调试器将暂停程序的位置——然后让程序一直运行到这个断点。如果不先设置断点就直接启动程序，程序会一口气运行到结束而不暂停，这样并没有多大用处。

我们在 main 的入口设置一个断点：

```text
(lldb) break set -n main
Breakpoint 1: where = hello_debugger`main, address = 0x0000000100003f65
```

注意，main 在你的机器上可能位于不同的内存地址。`break set` 命令创建一个新断点；`-n` 选项指定我们想要中断的函数名。稍后我们还会介绍设置断点的其他方式。

现在运行程序：

```text
(lldb) run
Process 6750 launched: '/Users/me/hello_debugger' (x86_64)
Process 6750 stopped
* thread #1, queue = 'com.apple.main-thread', ❶ stop reason = breakpoint 1.1
    frame #0: 0x0000000100003f65 hello_debugger`main
❷ hello_debugger`main:
❸ ->  0x100003f65 <+0>: pushq  %rbp
    0x100003f66 <+1>: movq   %rsp, %rbp
    0x100003f69 <+4>: subq   $0x10, %rsp
    0x100003f6d <+8>: callq  0x100003fb2               ; f
Target 0: (hello_debugger) stopped.
(lldb)
```

停止原因（stop reason）❶ 告诉我们，程序命中了刚刚设置的断点。LLDB 还贴心地告诉我们当前停在 hello_debugger 的 main 函数中 ❷，并打印出了接下来的几条汇编指令 ❸。

程序暂停之后，有几条命令可以让它继续执行：

`continue` 恢复程序运行，直到命中另一个断点或程序退出。

`finish` 恢复程序运行，并在从当前函数返回时再次暂停。

`stepi` 执行下一条指令，然后暂停。它会单步进入 call 指令，在被调用函数的第一条指令处暂停。命令 `stepi -c <n>` 会单步执行 n 条指令。

`nexti` 执行下一条指令，然后暂停。它会单步越过 call 指令，在当前函数中 call 之后的下一条指令处暂停。命令 `nexti -c <n>` 会单步执行 n 条指令。

大多数 LLDB 命令都有好几个别名。例如，`continue` 是 `process continue` 的快捷方式，它还可以进一步缩短为单字母命令 `c`。我们讲到的所有命令的完整形式和缩写形式见第 697 页的表 A-2。

我们来试试这些新命令。首先执行四条指令，这应该会让我们进入对 f 的调用：

```text
(lldb) stepi -c 4
--snip--
❶ hello_debugger`f:
->  0x100003fb2 <+0>: movl   $0x1, %eax
--snip--
```

从命令输出可以看出，我们停在 f 而不是 main 里 ❶。现在我们从 f 返回：

```text
(lldb) finish
--snip--
hello_debugger`main:
->  0x100003f72 <+13>: movl   $0x87654321, %eax         ; imm = 0x87654321
--snip--
```

我们回到了 main 中，位于 callq 紧后面的那条指令上。继续执行：

```text
(lldb) continue
Process 6750 resuming
Process 6750 exited with status = 0 (0x00000000)
```

由于没有再命中任何断点，程序一直运行到退出。要继续调试，我们就得重新启动它：

```text
(lldb) run
```

现在我们又暂停在 main 的开头。我们再次向前执行四条指令，不过这次用 `nexti` 越过 f，而不是进入它：

```text
(lldb) nexti -c 4
--snip--
hello_debugger`main:
->  0x100003f72 <+13>: movl   $0x87654321, %eax         ; imm = 0x87654321
--snip--
```

我们再次停在了 callq 紧后面的那条指令上。

#### 按地址设置断点

除了在函数上设置断点，还可以在具体的机器指令上设置断点。我们在指令 movl 0xdeadbeef, -4(%rbp) 上设一个断点。首先需要找到这条指令的地址。幸运的是，LLDB 已经把信息给出来了。上一条命令的输出应该像这样：

```text
hello_debugger`main:
->  0x100003f72 <+13>: movl   $0x87654321, %eax         ; imm = 0x87654321
    0x100003f77 <+18>: movsd  0x181(%rip), %xmm0        ; dbl, xmm0 = mem[0],zero
  ❶ 0x100003f7f ❷ <+26>: movl   $0xdeadbeef, -0x4(%rbp)   ; imm = 0xDEADBEEF
    0x100003f86 <+33>: movl   $0x0, -0x8(%rbp)
```

这里显示了接下来的几条指令，包括我们想要中断的那条。我们可以看到该指令的内存地址 ❶，以及该地址相对函数起始处的字节偏移量 ❷。确切的地址在你的机器上可能不同，但偏移量应该一致。要设置这个断点，输入

```text
(lldb) break set -a MEMORY_ADDRESS
```

其中 MEMORY_ADDRESS 是该指令在你机器上的地址。`-a` 选项表明我们指定的是一个地址而不是函数名。我们还可以用更复杂的表达式来指定指令地址。下面是在同一条指令上设置断点的另一种方式：

```text
(lldb) break set -a '(void()) main + 26'
```

首先，我们把 main 强制转换为某种函数类型，这样 LLDB 才能拿它做地址运算。（具体转换成哪种函数类型并不重要。）然后，我们加上 26 字节的偏移量，得到想要中断的那条 movl 指令的地址。由于这个地址表达式包含空格和特殊字符，我们必须把整个表达式用引号括起来。

稍后我们会看到如何反汇编整个函数、查看每条指令的地址。在此之前，先来看几条管理断点的其他实用命令。

#### 管理断点

我们来列出设置过的所有断点：

```text
(lldb) break list
Current breakpoints:
1: name = 'main', locations = 1, resolved = 1, hit count = 1
  1.1: where = hello_debugger`main, address = 0x0000000100003f65, resolved, hit count = 1

2: address = hello_debugger[0x0000000100003f7f], locations = 1, resolved = 1, hit count = 0
  2.1: where = hello_debugger`main + 26, address = 0x0000000100003f7f, resolved, hit count = 0

3: address = hello_debugger[0x0000000100003f7f], locations = 1, resolved = 1, hit count = 0
  3.1: where = hello_debugger`main + 26, address = 0x0000000100003f7f, resolved, hit count = 0
```

每个断点都有一个唯一的编号，需要删除、禁用或修改断点时可以用它来指代。在上一节中，我们把断点 2 和 3 设在了同一个位置 main+26。我们来删掉其中一个：

```text
(lldb) break delete 3
```

接下来，我们来看看如何显示函数中的全部汇编指令及其地址。

### 显示汇编代码

命令 `disassemble -n <function name>` 让 LLDB 打印函数中的所有汇编指令。我们在 main 上试一下：

```text
(lldb) disassemble -n main
hello_debugger`main:
    0x100003f65 <+0>:  pushq  %rbp
    0x100003f66 <+1>:  movq   %rsp, %rbp
    0x100003f69 <+4>:  subq   $0x10, %rsp
    0x100003f6d <+8>:  callq  0x100003fb2               ; f
->  0x100003f72 <+13>: movl   $0x87654321, %eax         ; imm = 0x87654321
    0x100003f77 <+18>: movsd  0x181(%rip), %xmm0        ; dbl, xmm0 = mem[0],zero
    0x100003f7f <+26>: movl   $0xdeadbeef, -0x4(%rbp)   ; imm = 0xDEADBEEF
    0x100003f86 <+33>: movl   $0x0, -0x8(%rbp)
    0x100003f8d <+40>: movl   $0xffffffff, -0xc(%rbp)   ; imm = 0xFFFFFFFF
    0x100003f94 <+47>: movl   $0xfeedface, -0x10(%rbp)  ; imm = 0xFEEDFACE
    0x100003f9b <+54>: movl   $0x19, %ecx
    0x100003fa0 <+59>: subl   $0x1, %ecx
    0x100003fa3 <+62>: cmpl   $0x0, %ecx
    0x100003fa6 <+65>: jne    0x100003fa0               ; <+59>
    0x100003fa8 <+67>: movl   $0x0, %eax
    0x100003fad <+72>: movq   %rbp, %rsp
    0x100003fb0 <+75>: popq   %rbp
    0x100003fb1 <+76>: retq
(lldb)
```

`->` 符号指向当前指令。我们还可以从指定地址开始打印固定数量的指令。我们来反汇编五条指令，从 main 的第三条指令开始。在这里显示的反汇编代码中，这条指令的地址是 0x100003f69；在你的机器上它可能不同。`-s` 选项指定 LLDB 从哪个地址开始反汇编，`-c` 指定显示多少条指令，因此我们用下面这条命令反汇编这五条指令：

```text
(lldb) disassemble -s 0x100003f69 -c 5
hello_debug`main:
    0x100003f69 <+4>:  subq   $0x10, %rsp
    0x100003f6d <+8>:  callq  0x100003fb2               ; f
->  0x100003f72 <+13>: movl   $0x87654321, %eax         ; imm = 0x87654321
    0x100003f77 <+18>: movsd  0x181(%rip), %xmm0        ; dbl, xmm0 = mem[0],zero
    0x100003f7f <+26>: movl   $0xdeadbeef, -0x4(%rbp)   ; imm = 0xDEADBEEF
```

最后，我们可以用 `--pc` 选项从当前指令开始反汇编：

```text
(lldb) disassemble --pc -c 3
->  0x100003f72 <+13>: movl   $0x87654321, %eax         ; imm = 0x87654321
    0x100003f77 <+18>: movsd  0x181(%rip), %xmm0        ; dbl, xmm0 = mem[0],zero
    0x100003f7f <+26>: movl   $0xdeadbeef, -0x4(%rbp)   ; imm = 0xDEADBEEF
```

这条命令显示三条指令，从当前指令开始。当用 `-s` 或 `--pc` 指定起始地址时可以使用 `-c` 选项，但在用 `-n` 反汇编整个函数时不能使用。

### 打印表达式

你可以用命令 `exp -f <format> -- <expr>` 对表达式求值，其中：

`<format>` 是格式说明符，告诉 LLDB 如何显示表达式的结果。

`<expr>` 是任意表达式。表达式既可以引用正在运行的程序中的寄存器、内存地址和符号，也可以包含 C 的运算，比如算术运算、指针解引用和强制类型转换。

我们来看几个例子。现在程序应该停在指令 movl 0x87654321, %eax 上。我们先单步执行这条指令，然后用几种不同的格式打印 EAX 的值：

```text
(lldb) stepi
--snip--
hello_debugger`main:
->  0x100003f77 <+18>: movsd  0x181(%rip), %xmm0        ; dbl, xmm0 = mem[0],zero
--snip--
(lldb) exp -- $eax
(unsigned int) $0 = ❶ 2271560481
(lldb) exp -f x -- $eax
(unsigned int) $1 = ❷ 0x87654321
(lldb) exp -f d -- $eax
(unsigned int) $2 = ❸ -2023406815
```

默认情况下，LLDB 把通用寄存器中的值格式化为无符号整数 ❶。这里我们又以十六进制 ❷ 和有符号整数 ❸ 的形式显示了 EAX 中的值。（完整的格式列表可以用 `help format` 命令查看。）符号 $0、$1 等是便利变量（convenience variable），LLDB 会自动生成它们来保存每个表达式的求值结果。

第 13 章介绍了保存浮点值的 XMM 寄存器。程序中的下一条指令 movsd dbl(%rip), %xmm0 会把静态变量 dbl 中的值 3.5 复制到 XMM0 中。我们单步执行这条指令，然后检查 XMM0。我们将使用 float64[] 格式，它把寄存器内容显示为两个 double 组成的数组：

```text
(lldb) stepi
--snip--
hello_debugger`main:
->  0x100003f7f <+26>: movl   $0xdeadbeef, -0x4(%rbp)   ; imm = 0
--snip--
(lldb) exp -f float64[] -- $xmm0
(unsigned char __attribute__((ext_vector_type(16)))) $3 = ( ❶ 3.5, 0)
```

数组的第一个元素对应寄存器的低 64 位 ❶，也就是我们用 movsd 指令更新的部分；第二个元素对应寄存器的高 64 位，可以忽略。

除了寄存器，我们还可以打印符号表中对象的值。我们来检查这个程序里的两个静态变量 integer 和 dbl：

```text
(lldb) exp -f d -- integer
(void *) $4 = 100
(lldb) exp -f f -- dbl
(void *) $5 = 3.5
```

再来看几个更复杂表达式的例子。我们可以做基本的算术运算：

```text
(lldb) exp -f x -- $eax + 0x10
(unsigned int) $6 = 0x87654331
```

我们可以调用当前程序或标准库中的函数。这里我们调用 f，它返回 1：

```text
(lldb) exp -- (int) f()
(int) $7 = 1
```

我们还可以解引用指针。先执行下一条指令 movl 0xdeadbeef, -4(%rbp)，然后检查 -4(%rbp) 处的值：

```text
(lldb) stepi
--snip--
hello_debugger`main:
->  0x100003f86 <+33>: movl   $0x0, -0x8(%rbp)
--snip--
(lldb) exp -f x -- *(int *)($rbp - 4)
(int) $8 = 0xdeadbeef
```

首先，我们算出想要检查的内存地址 $rbp - 4；然后把这个地址强制转换成正确的指针类型 (int *)；最后用解引用运算符 * 对它解引用。这会得到一个整数，我们用选项 -f x 以十六进制把它打印出来。

接下来，我们来看一种更灵活的检查内存中数据的方法。

### 检查内存

我们可以用 `memory read` 命令检查内存。与 exp 一样，它接受任意表达式，该表达式的求值结果必须是有效的内存地址。这给了我们另一种检查 -4(%rbp) 处整数的方法：

```text
(lldb) memory read -f x -s 4 -c 1 '$rbp - 4'
0x3040bb93c: 0xdeadbeef
```

选项 `-f x` 表示以十六进制打印输出；`-s 4` 表示把内存内容解释为一系列 4 字节的值；`-c 1` 表示只打印其中一个值。换句话说，这条命令打印 $rbp - 4 处单个的 4 字节整数，并以十六进制格式化。表达式 $rbp - 4 里包含空格，所以必须用引号括起来。

清单 A-1 中接下来的三条指令又往栈上存了三个整数：

```asm
    movl    $0, -8(%rbp)
    movl    $-1, -12(%rbp)
    movl    $0xfeedface, -16(%rbp)
```

我们来单步执行这三条指令，然后打印出整个栈帧。我们让 LLDB 从 RSP 中的地址开始打印六个 4 字节的字，并使用选项 `-l 1` 让每个字单独占一行：

```text
(lldb) stepi -c 3
--snip--
hello_debugger`main:
->  0x100003f9b <+54>: movl   $0x19, %ecx
--snip--
(lldb) memory read -f x -s 4 -c 6 -l 1 $rsp
0x3040bb930: ❶ 0xfeedface
0x3040bb934: 0xffffffff
0x3040bb938: 0x00000000
0x3040bb93c: 0xdeadbeef
0x3040bb940: ❷ 0x040bba50
0x3040bb944: 0x00000003
```

之所以打印六个字，是因为这个函数的栈共 24 字节：在 main 开头，我们把 RBP 的旧值保存到了栈上，占 8 字节；随后又用指令 subq $16, %rsp 分配了另外 16 字节。请记住，RSP 始终保存着栈顶的地址，也就是最低的栈地址。

这条命令向我们展示了保存到栈上的四个整数：0xfeedface 在最顶端 ❶，RBP 的旧值在最底端 ❷。由于 ❷ 处的值实际上是一个 8 字节的地址，把栈按 8 字节分组来读会更容易看清：

```text
(lldb) memory read -f x -s 8 -c 3 -l 1 $rsp
0x3040bb930: 0xfffffffffeedface
0x3040bb938: 0xdeadbeef00000000
0x3040bb940: ❶ 0x00000003040bba50
```

现在可以清楚地看到，栈最底部的 8 字节保存的是一个完整的内存地址 ❶。

在保存的 RBP 值的正下方、落在调用者栈帧最顶端的位置，我们预期会找到调用者的返回地址——也就是我们从 main 返回时将要跳转到的地址。（第 9 章实现函数调用时我们详细讨论过这一点。）我们来检查这个地址：

```text
(lldb) memory read -f A -s 8 -c 4 -l 1 $rsp
0x3040bb930: 0xfffffffffeedface
0x3040bb938: 0xdeadbeef00000000
0x3040bb940: 0x00000003040bba50
0x3040bb948: ❶ 0x0000000200012310 dyld`start + 2432
```

这条命令与上一条几乎相同，只是用选项 `-c 4` 打印四个值而不是三个，并用选项 `-f A` 把每个值格式化为内存地址。A 格式说明符让 LLDB 以十六进制打印每个地址，并且（如果可能的话）打印出该地址相对程序中最近符号的偏移量。由于函数名和静态变量名都定义在符号表中，LLDB 能够显示汇编指令和静态数据的相对偏移量。至于栈地址、堆地址和无效地址，它则不会显示相对偏移量，因为那样做毫无意义。

输出的前三行与之前相同。前两个值不是有效的内存地址，第三个是栈地址，所以 LLDB 无法显示它们相对符号的偏移量。栈上的下一个值就是返回地址 ❶。标签 dyld`start 告诉我们，这是 dyld 共享库中 start 函数里一条指令的地址。（start 函数负责调用 main 并在其退出后做清理；dyld 是动态链接器。）

`-f A` 选项让我们很容易认出返回地址和指向静态变量的指针。如果你的程序栈帧被破坏了，这一点尤其有用；找到每个栈帧的返回地址能帮你重新理清头绪。

### 设置条件断点

作为本次演练的收尾，我们来看看如何设置条件断点。只有当关联的条件为真时，程序才会在条件断点处暂停。条件可以是任意表达式；LLDB 把求值结果为 0 的条件视为假，其他情况视为真。

我们在 hello_debugger 中最后一次循环迭代末尾的 jne 指令上设置一个断点。首先，在反汇编出的 main 函数里找到这条指令的地址：

```text
(lldb) disassemble -n main
hello_debugger`main:
    --snip--
  ❶ 0x100003fa6 <+65>:  jne    0x100003fa0               ; <+59>
    --snip--
```

这里 jne 的地址是 0x100003fa6 ❶。现在我们设置一个条件断点，当 ECX 为 0 时在这条 jne 指令处暂停。我们可以用 `-c` 选项指定条件：

```text
(lldb) break set -a MEMORY_ADDRESS -c '$ecx == 0'
```

由于这个循环会一直重复到 ECX 变为 0，条件 $ecx == 0 只会在最后一次迭代时为真。我们继续运行到断点，然后验证条件确实为真：

```text
(lldb) continue
--snip--
hello_debugger`main:
->  0x100003fa6 <+65>:  jne    0x100003fa0               ; <+59>
--snip--
(lldb) exp -- $ecx
(unsigned int) $9 = 0
```

如果你得到的 ECX 值不是这个，请检查断点是否设置正确：

```text
(lldb) break list
--snip--
4: address = hello_debugger[0x0000000100003fa6], locations = 1, resolved = 1, hit count = 0
Condition: $ecx == 0 ❶

  4.1: where = ❷ hello_debugger`main + 65, address = 0x0000000100003fa6, resolved, hit count
 = 0
```

请确认你的断点带有条件 $ecx == 0 ❶，并且位于 hello_debugger`main + 65 处 ❷。如果你的断点不是这样，可能是哪里输错了；删掉它重新再试。

现在应该处于最后一次循环迭代，我们向前单步执行一条指令，确认跳转没有发生：

```text
(lldb) stepi
--snip--
hello_debugger`main:
->  0x100003fa8 <+67>:  movl   $0x0, %eax
--snip--
```

通常 jne 会跳回循环开头，但在最后一次迭代中，它会向前推进到下一条指令。

### 获取帮助

要了解更多这里没有讲到的命令和选项，请参阅 LLDB 文档：https://lldb.llvm.org/index.html。你也可以在提示符下输入 `help` 来了解任何 LLDB 命令的更多信息。例如，要查看 `run` 命令的文档，可以输入：

```text
(lldb) help run
     Launch the executable in the debugger
--snip--
```

表 A-2 总结了我们介绍过的命令和选项。表中首先列出演练中使用的命令形式，然后是更短的缩写（exp 除外，它通常不再缩短），最后是与演练所用形式不同的完整形式。每个命令的所有形式接受的参数都相同。

*表 A-2：LLDB 命令总结*

| 命令 | 缩写 | 完整形式 | 描述 |
| --- | --- | --- | --- |
| run | r | process launch -- | 启动程序。 |
| continue | c | process continue | 恢复程序运行。 |
| finish | fin | thread step-out | 恢复程序运行并一直执行到当前函数退出。 |
| stepi [-c <n>] | si | thread step-inst | 执行一条（或 n 条）指令，单步进入函数调用。 |
| nexti [-c <n>] | ni | thread step-inst-over | 执行一条（或 n 条）指令，单步越过函数调用。 |
| break set [-n <fun> \| -a <addr>] [-c <cond>] | br s | breakpoint set | 在函数 <fun> 的起始处或地址 <addr> 处设置断点（若提供了 <cond>，则以 <cond> 为条件）。 |
| break list | br l | breakpoint list | 列出所有断点。 |
| break delete [<id>] | br del | breakpoint delete | 删除所有断点（或由 <id> 指定的断点）。 |
| exp -f <format> -- <expr> | — | expression | 对 <expr> 求值，并以格式 <format> 显示结果。 |
| memory read -f <format> -s <size> -c <count> -l <num-per-line> <addr> | me read | — | 从地址 <addr> 开始打印 <count> 个大小为 <size> 字节的内存块，每行显示 <num-per-line> 个内存块，格式为 <format>。 |
| disassemble [-n <fun> \| -s <addr> -c <count> \| --pc -c <count>] | di | — | 反汇编函数 <fun> 中的所有指令，或从地址 <addr> 开始的 <count> 条指令，或从当前指令开始的 <count> 条指令。 |
| help <cmd> | h | — | 显示关于 <cmd> 的帮助文本。 |

现在，你已经可以用 LLDB 开始调试了！
