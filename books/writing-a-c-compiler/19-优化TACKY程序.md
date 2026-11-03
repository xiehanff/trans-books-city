# 第三部分　优化（Part III: Optimizations）

# 第 19 章　优化 TACKY 程序

*Chapter 19: Optimizing TACKY Programs*

在本书的前两部分中，你编写了一个支持 C 语言大部分特性的编译器。你确保了生成的可执行程序是正确的——换句话说，它们的行为符合 C 标准——但你并没有担心它们的性能。你没有设法让它们跑得更快、占用更少的存储空间或消耗更少的内存。在第三部分中，你将专注于优化这些程序——也就是说，在不改变其行为的前提下，让它们更小、更快。

有些编译器优化是机器无关的（machine-independent），意思是它们不受目标体系结构细节的影响，比如可用寄存器的数量或特定汇编指令所受的约束。编译器通常会在把程序转换为汇编之前，先在 TACKY 这样的中间表示（intermediate representation，IR）上执行这些优化。而机器相关的（machine-dependent）优化则需要把目标体系结构考虑进来，因此通常在程序被转换为汇编之后才进行。本章介绍四种广泛使用的机器无关优化：常量折叠（constant folding）、不可达代码消除（unreachable code elimination）、复制传播（copy propagation）和死存储消除（dead store elimination）。你将添加一个新的优化阶段（在本章开头的示意图中以粗体标出），把这四种优化应用到 TACKY 程序上。下一章将介绍寄存器分配（register allocation），它是一种机器相关的优化。

在开始第三部分之前，你不需要先完成第二部分。对于每种优化，我们都先给出一个不考虑第二部分语言特性的实现；然后若有必要，再把它扩展到支持那些特性——如果你没有做第二部分，可以跳过这一步。除了不可达代码消除（它不受第二部分特性的影响）之外，其他每种优化都需要这个额外步骤。

这两章只涵盖生产级编译器中众多优化的一小部分，但我们要介绍的基本概念同样适用于许多其他优化。在开始之前，先来考虑一个对所有编译器优化都至关根本的问题：我们怎么知道优化后的代码是正确的？

## 安全性与可观察行为

首先也是最重要的，编译器优化必须是安全的（safe），也就是说它们不能改变程序的语义。（如果程序行为不正确，跑得再快也没用！）具体而言，优化不得改变程序的可观察行为（observable behavior），即对其执行环境可见的行为。返回退出状态、向 stdout 打印消息、写入文件，都是可观察行为的例子。程序执行的大多数动作——比如计算值、更新局部变量、把控制权从一条语句转移到另一条语句——对执行环境都不可见，因此只是间接影响程序的可观察行为。这给了我们变换程序的很大灵活性。只要可观察行为不变，我们就可以重排、替换甚至删除代码。

来看看 GCC 如何优化一个简单的 C 程序。清单 19-1 用 1、2、3 初始化三个变量，把它们相加后返回结果。

```c
int main(void) {
    int x = 1;
    int y = 2;
    int z = 3;
    return x + y + z;
}
```

*清单 19-1：一个把三个变量相加的 C 程序*

这个程序每次运行时的可观察行为都相同：以退出状态 6 终止。你可以运行下面的命令在关闭优化的情况下编译该程序：

```bash
$ gcc -S -fno-asynchronous-unwind-tables -fcf-protection=none listing_19_1.c
```

这会生成清单 19-2（或与之类似）的汇编代码。

```asm
    .text
    .globl main
main:
    pushq   %rbp
    movq    %rsp, %rbp
    movl    $1, -4(%rbp)
    movl    $2, -8(%rbp)
    movl    $3, -12(%rbp)
    movl    -4(%rbp), %edx
    movl    -8(%rbp), %eax
    addl    %eax, %edx
    movl    -12(%rbp), %eax
    addl    %edx, %eax
    popq    %rbp
    ret
```

*清单 19-2：清单 19-1 未优化的汇编代码*

这段汇编程序忠实地实现了清单 19-1 的源代码：它在栈上初始化三个存储位置，值为 1、2、3；把它们相加；然后在 EAX 中返回结果。现在我们用 -O 开关编译同一份源代码，启用优化：

```bash
$ gcc -S -O -fno-asynchronous-unwind-tables -fcf-protection=none listing_19_1.c
```

这会生成清单 19-3 中的汇编代码。

```asm
    .text
    .globl main
main:
    movl    $6, %eax
    ret
```

*清单 19-3：清单 19-1 优化后的汇编代码*

这段汇编程序不再初始化三个变量再相加，而是直接返回常量 6。清单 19-2 和清单 19-3 看起来差别很大，但它们都产生了正确的可观察行为。

**较真者的可观察行为**

关于可观察行为，更正式的定义可以在 C 标准的 5.1.2.3 节第 6 段找到。这一节列出了三类可观察行为：对文件的操作、对交互设备（interactive device）的操作，以及对 volatile 对象的访问。C 程序可以使用标准库中的 I/O 函数操作文件和交互设备。交互设备通常是终端之类的东西，但其确切含义由实现定义。基本思想是：写往文件的输出可以被缓冲，而发往交互设备的输出必须立即送达。我们在本章实现的优化会保留所有函数调用，包括那些操作文件和交互设备的标准库函数调用。

volatile 对象（volatile object）是以编译器无法理解的方式与执行环境交互的对象。例如，它可能被信号处理程序读取，或用于通过内存映射 I/O（memory-mapped I/O）与硬件设备通信。声明 volatile 对象要使用 volatile 类型限定符（type qualifier）。我们没有实现这个限定符，所以不必担心 volatile 访问。

C 标准对可观察行为的定义并不力求穷尽；各个实现可以（而且通常确实）对程序行为提供更强的保证。所以这个定义遗漏了许多你可能以为可观察的行为（比如启动新进程和其他非 I/O 系统调用），是可以理解的。更令人意外的是，它还遗漏了标准本身其他部分暗示应当可观察的一些行为。例如，按照这个定义，程序的退出状态不属于可观察行为，但 5.1.2.2.3 节又说退出状态应当"返回给宿主环境"。这种遗漏也许有充分的理由，但在我看来这是标准的一个问题。

实践中，没有任何 C 编译器会执行可能改变程序退出状态的优化。我们的编译器也不例外；它的优化会保留每个函数调用的返回值，包括 main 返回的退出状态。

## 四种 TACKY 优化

本节介绍本章要实现的优化：常量折叠、不可达代码消除、复制传播和死存储消除。这些优化的目标是让代码更快、占用空间更少。就单个优化而言，有的能促进其中一个或两个目标，有的单独作用并不明显。真正的收益来自它们协同工作的方式，因为运行其中任何一个都会为应用其他三个创造新机会。我们将依次考察这四种优化，然后讨论每一种如何让其他优化更有效。

动手之前请注意：在本章的大部分 TACKY 代码清单中，我会使用极简记法。我会像前几章偶尔做过的那样，把复制写成 x = y 而不是 Copy(Var("x"), Var("y"))，并对其他指令采取类似的简写。例如，二元运算写成 x = a + b 而不是 Binary(Add, Var("a"), Var("b"), Var("x"))，标签写成 Target: 而不是 Label(Target)。这种记法让我们把注意力放在 TACKY 程序的高层逻辑上，而不是每条 TACKY 指令的细节上。

### 常量折叠

常量折叠遍在编译期对常量表达式求值。例如，常量折叠会把 Binary TACKY 指令

```text
a = 6 / 2
```

替换为一条 Copy 指令：

```text
a = 3
```

常量折叠还能把条件跳转变成无条件跳转，或者干脆消除它们。它会把

```text
JumpIfZero(0, Target)
```

变换为

```text
Jump(Target)
```

因为程序总会执行这个跳转。它还会删除指令

```text
JumpIfZero(1, Target)
```

因为程序永远不会执行这个跳转。（删除无用跳转通常被视为死代码消除（dead-code elimination）而非常量折叠，但反正这一遍就要处理条件跳转，不如顺手把无用的也删掉。）

常量折叠对速度和代码大小都有帮助。单个算术运算或比较可能需要多条汇编指令，其中一些（比如 idiv）相当慢。常量折叠最终会用一条 mov 指令替换那整段汇编代码。

### 不可达代码消除

不可达代码消除删除那些我们确知永远不会执行的指令。考虑清单 19-4 中的 TACKY 片段。

```text
x = 5
Jump(Target)
x = my_function()
Target:
Return(x)
```

*清单 19-4：包含一条不可达指令的 TACKY 片段*

既然我们总会跳过对 my_function 的调用，可以把它删掉：

```text
x = 5
Jump(Target)
Target:
Return(x)
```

现在 Jump 指令没用了，因为它跳到的是我们接下来本来就要执行的指令。我们把它也删掉：

```text
x = 5
Target:
Return(x)
```

最后，假设没有其他指令跳转到 Target 标签，我们还可以把这个标签也消除掉：

```text
x = 5
Return(x)
```

严格来说，我们刚删掉的 Jump 和 Label 指令并不属于不可达代码；运行中的程序会执行到它们，只是它们不产生任何效果。但删除不可达代码常常会让跳转和标签变得无用，所以在这一遍里顺手删除它们是顺理成章的。

消除不可达代码显然能减小代码体积。删除无用跳转能节省时间也同样明显；哪怕一条无用指令也要花一点执行时间。事实证明，删除真正不可达的指令（比如清单 19-4 中的 FunCall）也能加速程序，因为这能降低内存压力、释放处理器指令缓存（instruction cache）中的空间。

另一方面，删除无用标签不会影响速度或代码大小，因为标签在最终的可执行文件中不会变成机器指令。我们仍然会删除这些标签，因为这能让 TACKY 程序更易读、易调试，而且几乎不需要额外的工作。

这一遍特别适合清理我们在每个 TACKY 函数末尾额外添加的那条 Return 指令。回想一下，我们添加这条指令是为了兜底，以防源代码缺少 return 语句。把程序转换为 TACKY 时，我们无法判断这条额外的 Return 是否必要，于是最终把它也加进了并不需要它的函数。不可达代码消除遍会删除所有我们多余添加的 Return 指令，同时保留真正需要的那些。这是一个更普遍原则的例子：先生成低效代码再事后优化，往往比一开始就生成高效代码更容易。

### 复制传播

当程序中包含 Copy 指令 dst = src 时，复制传播遍会尝试在后续指令中用 src 替换 dst。看下面这段 TACKY：

```text
x = 3
Return(x)
```

我们可以在 Return 指令中把 x 替换为它当前的值 3：

```text
x = 3
Return(3)
```

用常量替换变量是复制传播的一个特例，称为常量传播（constant propagation）。其他情况下，我们会用一个变量替换另一个变量。例如，可以把

```text
x = y
Return(x)
```

重写为：

```text
x = y
Return(y)
```

有时，判断执行复制传播是否安全并不容易。看这个例子：

```text
x = 4
JumpIfZero(flag, Target)
x = 3
Target:
Return(x)
```

到达 Return 指令时，x 的值取决于我们走哪条路径，可能是 3 也可能是 4。由于不知道程序会沿哪条路径到达这条指令，我们无法安全地把 x 替换为其中任何一个值。要处理这类情况，就需要分析通往我们想改写的那条指令的每一条可能路径。我们将使用一种叫数据流分析（data-flow analysis）的技术来考察函数中所有路径，找出能安全执行复制传播的位置。数据流分析不只对复制传播有用；它被用于许多不同的编译器优化，包括接下来要讨论的死存储消除。

我们分析的一些复制会涉及静态存储期（static storage duration）的变量，它们可以被多个函数访问（对于局部静态变量来说，则是同一函数的多次调用）。我们并不总能确切判断这些变量何时被更新，所以数据流分析需要把它们与自动存储期（automatic storage duration）的变量区别对待。如果你完成了第二部分，还需要对用 & 运算符取了地址的变量考虑类似的不确定性，因为它们可以通过指针被更新。

复制传播本身没什么用，但它能让其他优化更有效。传播常量会为常量折叠创造新机会；有时我们会把 Copy 指令目的地的所有使用都替换成它的源，这会让这条 Copy 本身变得无用。我们将在最后一个优化遍——死存储消除——中删除这些无用指令。

### 死存储消除

当一条指令更新了变量的值，而我们从不使用那个新值时，这条指令就称为死存储（dead store）。（这里的"存储"泛指任何把值存入变量的指令，而不是第二部分引入的 TACKY Store 指令。）由于死存储不影响程序的可观察行为，删除它们是安全的。看一个简单的例子：

```text
x = 10
Return(y)
```

假设 x 具有自动存储期，指令 x = 10 就是一个死存储。从这条指令到函数末尾（也就是 x 生命周期的结束）之间，我们都没有使用 x。

下面是另一种类型的死存储：

```text
x = a + b
x = 2
Return(x)
```

在这个例子中，a + b 的结果永远不会被使用，因为它会先被覆盖；这意味着 x = a + b 是一个死存储。死存储消除遍会识别并删除这样的无用指令。难点在于证明一条指令确实是死存储；为此，我们需要分析函数中的每一条路径，确保它赋给目的地的值从未被使用。我们再次借助数据流分析来判断何时可以安全地应用这个优化。

与复制传播一样，一旦把可被多个函数访问或可通过指针访问的对象考虑进来，死存储消除就变得更复杂。例如，如果 x 是全局变量，第一个例子中的指令 x = 10 就不是死存储；函数返回后 x 可能还会被使用。我们的数据流分析必须把这种可能性考虑进来。

**当优化发起攻击时！**

从不改变程序可观察行为的意义上说，死存储消除是安全的。但从另一种更直观的意义上说，它不安全：它可能降低程序的安全性。一个认真对待安全的程序员会在处理完敏感数据后立即将其覆盖。秘密在内存中停留得越久，攻击者读到它的风险就越大——也许是通过利用程序中的另一个漏洞，甚至是转储整个系统的内存。不幸的是，清除敏感数据的操作往往是死存储。而死存储消除嘛，自然倾向于消除它们。考虑下面这个试图把加密密钥清零的代码片段：

```c
char *encryption_key = malloc(encryption_key_size);
// 初始化 encryption_key 并用它加密一些内容
--snip--
memset(encryption_key, 0, encryption_key_size);
free(encryption_key);
return 0;
```

正常情况下，调用 memset 会把 encryption_key 指向的缓冲区清零，防止缓冲区中的数据日后泄露。但编译器可能会把对 memset 的调用优化掉，因为严格来说它是一个死存储；它只是更新了一个我们再也不会使用的缓冲区。我们的死存储消除实现不会从这个例子中删除 memset，因为它从不优化掉函数调用，但 GCC 和 Clang 确实会在这类代码中消除 memset。你可以编译一个小例子亲自验证。

C 程序可以通过使用专门的库函数来清除内存以避免这个问题，编译器知道不该把这些函数优化掉。有几个服务于这一目的的平台专用函数，比如 Windows 上的 SecureZeroMemory 和许多 Linux 发行版上的 explicit_bzero。用于安全写内存的 memset_s 函数早在 C11 版本的标准中就被加入了 C 标准库，但它属于一个可选附录，从未被广泛实现。C23 引入了一个类似的、非可选的函数 memset_explicit，C 程序员终于有了一种标准且可移植的清除内存中敏感数据的方法。

memset_explicit 这类函数解决了死存储带来的直接问题。但它们没有触及更根本的问题：可观察行为的概念并不能涵盖程序员关心的所有行为。C 标准保证，在一切正常时——你的代码没有未定义行为、它依赖的库没有未定义行为、也没有人篡改底层系统——你的代码会按你的意图运行；但对于出问题时会发生什么，它没有任何保证。没有这些保证，编写安全的代码很困难，有时甚至不可能。（如果你想进一步了解编译器优化在安全方面的影响，见第 610 页的「延伸资源」，其中给出了几篇相关论文的链接。）

我们不必担心这里实现的优化对安全的影响。如果你在用为本书编写的编译器去编译安全攸关的软件，死存储消除是你所有问题中最不足挂齿的一个。

### 合众之力……

现在来看看本章要实现的四种优化如何协同工作。我们用清单 19-5 中的 TACKY 程序作为贯穿示例。

```text
my_function(flag):
    x = 4
    y = 4 - x
    JumpIfZero(y, Target)
    x = 3
    Target:
    JumpIfNotZero(flag, End)
    z = 10
    End:
    z = x + 5
    Return(z)
```

*清单 19-5：一个未优化的 TACKY 程序*

借助全部四种优化，我们可以把这个函数化简成一条 Return 指令。我会展示每一轮优化的结果，并突出显示发生变化的指令。由于每种优化都能为应用其他三种创造更多机会，要完全优化这个函数，大多数优化都得运行好几轮。眼下，我们每一步靠临场判断来选择运行哪种优化——看看代码，哪种最有用就用哪种。等到实际实现优化流水线时，我们会采用更系统的方法。

先来一轮复制传播，把 y = 4 - x 中的 x 替换为 4：

```text
my_function(flag):
    x = 4
    y = 4 - 4
    JumpIfZero(y, Target)
    x = 3
    Target:
    JumpIfNotZero(flag, End)
    z = 10
    End:
    z = x + 5
    Return(z)
```

我们不能替换 x 的第二次使用（z = x + 5 中的那个），因为在那一时刻 x 可能的值不止一个：取决于是否走条件跳转，它可能是 3 也可能是 4。接下来，对 y = 4 - 4 应用常量折叠：

```text
my_function(flag):
    x = 4
    y = 0
    JumpIfZero(y, Target)
    x = 3
    Target:
    JumpIfNotZero(flag, End)
    z = 10
    End:
    z = x + 5
    Return(z)
```

用 Copy 指令替换二元运算之后，我们又创造了一个复制传播的机会。可以把 JumpIfZero 指令中的 y 替换为它的值 0：

```text
my_function(flag):
    x = 4
    y = 0
    JumpIfZero(0, Target)
    x = 3
    Target:
    JumpIfNotZero(flag, End)
    z = 10
    End:
    z = x + 5
    Return(z)
```

现在 JumpIfZero 的条件是常量，我们可以再跑一次常量折叠，把它变成无条件 Jump：

```text
my_function(flag):
    x = 4
    y = 0
    Jump(Target)
    x = 3
    Target:
    JumpIfNotZero(flag, End)
    z = 10
    End:
    z = x + 5
    Return(z)
```

这一改动让 x = 3 变得不可达，于是运行不可达代码消除把它删掉。这一遍还会删除 Jump 指令和 Target 标签，因为删掉 x = 3 之后它们不再起任何作用：

```text
my_function(flag):
    x = 4
    y = 0
    JumpIfNotZero(flag, End)
    z = 10
    End:
    z = x + 5
    Return(z)
```

之前我们不能改写 z = x + 5，因为通往该指令的不同路径上 x 有两个不同的值。刚才通过消除经由 x = 3 的路径，我们解决了这个问题。现在可以再跑一次复制传播：

```text
my_function(flag):
    x = 4
    y = 0
    JumpIfNotZero(flag, End)
    z = 10
    End:
    z = 4 + 5
    Return(z)
```

然后再来一轮常量折叠：

```text
my_function(flag):
    x = 4
    y = 0
    JumpIfNotZero(flag, End)
    z = 10
    End:
    z = 9
    Return(z)
```

最后再跑一次复制传播：

```text
my_function(flag):
    x = 4
    y = 0
    JumpIfNotZero(flag, End)
    z = 10
    End:
    z = 9
    Return(9)
```

我们成功地在编译期算出了这个函数的返回值，顺带消除了 x、y、z 的所有使用。现在运行死存储消除，清理给这三个变量赋值的指令：

```text
my_function(flag):
    JumpIfNotZero(flag, End)
    End:
    Return(9)
```

最后，运行不可达代码消除，删掉 JumpIfNotZero 指令和 End 标签。它们如今都是多余的，因为我们刚刚消除了 JumpIfNotZero 所要跳过的唯一一条指令。这最后一轮优化会把我们的函数化简成一条指令：

```text
my_function(flag):
    Return(9)
```

这个例子展示了我们的优化相互配合的几种方式。复制传播可能用常量替换变量，为常量折叠创造新机会；常量折叠把算术运算改写为 Copy 指令，又为复制传播创造新机会。常量折叠能把条件跳转替换为无条件跳转，使某些指令不可达；消除不可达代码简化了程序的控制流，进而促进复制传播。复制传播可能让 Copy 指令变得多余，从而可以在死存储消除中删除它们。而死存储消除有可能删掉一条跳转与它所跳转到的标签之间的所有指令，这使得该跳转（可能还有该标签）成为不可达代码消除的候选。

现在我们知道了每种优化的作用以及它们如何协同工作。接下来，我们要添加几个新的命令行选项，以便测试它们。

## 测试优化遍

本章测试的工作方式与前几章不同。我们需要验证优化没有改变程序的可观察行为，同时确实化简了常量表达式、删除了无用代码。我们目前的策略——编译 C 程序、运行它们、确认行为正确——满足第一个要求，但不满足第二个。仅仅运行程序无法告诉你优化阶段到底做没做事。针对第二点，测试脚本会检查你的编译器为每个测试程序生成的汇编输出；针对第一点，它还会像前几章那样运行每个测试程序并验证其行为。

为了支持本章的测试，你需要给编译器添加几个命令行选项：

- `-S`：让编译器生成汇编文件，但不汇编或链接。运行 ./YOUR_COMPILER -S /path/to/program.c 应当把汇编文件写到 /path/to/program.s。（我在第 1 章就建议添加这个选项以便调试；如果还没加，现在得加上。）
- `--fold-constants`：启用常量折叠。
- `--propagate-copies`：启用复制传播。
- `--eliminate-unreachable-code`：启用不可达代码消除。
- `--eliminate-dead-stores`：启用死存储消除。
- `--optimize`：启用全部四种优化。

用于启用优化的选项应当传递给优化阶段（我们接下来就实现它）。应当可以同时启用多个单独的优化；例如，./YOUR_COMPILER --fold-constants --propagate-copies 应当同时启用常量折叠和复制传播，但不启用另外两种优化。

如果你的编译器生成汇编的方式与我在书中给出的不完全一致，测试脚本应该仍能验证本章测试的汇编输出，但有两点需要注意。第一，测试脚本只认识 AT&T 汇编语法，也就是我们全书使用的语法。第二，脚本并不能识别每一条汇编指令；它只知道本书用过的指令，以及现实汇编代码中特别常见的少数其他指令。如果你生成了测试脚本不认识的指令，某些测试可能会失败。

接下来，我们把新的优化阶段接进来，由它控制何时调用每个单独的优化。

## 接入优化阶段

优化阶段在程序转换为 TACKY 之后立即运行。这个阶段会独立地优化每个 TACKY 函数，完全不知道程序中定义的其他函数。例如，常量折叠遍不会尝试对函数调用求值，死存储消除也不会删除函数调用。（死存储消除不能删除函数调用，因为我们不知道它们是否有副作用。不过不可达代码消除可以删除它们——如果一个函数调用永远不会执行，函数有什么副作用都无所谓。）这类一次变换一个函数的优化称为过程内优化（intraprocedural optimization）。大多数生产级编译器还会执行过程间优化（interprocedural optimization），它变换的是整个翻译单元而非单个函数。

每个单独的优化都以 TACKY 函数体为输入，返回语义等价的函数体作为输出。在常量折叠遍中，我们照常把函数体表示为 TACKY 指令列表。而在其他三个优化遍中，我们把每个函数表示为控制流图（control-flow graph）。这是一种显式建模一段代码中不同执行路径的中间表示。本章稍后会详细讨论如何构造控制流图以及它们为什么有用。

优化阶段处理每个函数的方式是反复运行所有已启用的优化，直到达到不动点（fixed point）——再跑一遍也不会让函数进一步变化——才停止。清单 19-6 给出了这个优化流水线。

```text
optimize(function_body, enabled_optimizations):
    if function_body is empty:
        return function_body

    while True:
        if enabled_optimizations contains "CONSTANT_FOLDING":
          ❶ post_constant_folding = constant_folding(function_body)
        else:
            post_constant_folding = function_body

      ❷ cfg = make_control_flow_graph(post_constant_folding)

        if enabled_optimizations contains "UNREACHABLE_CODE_ELIM":
            cfg = unreachable_code_elimination(cfg)

        if enabled_optimizations contains "COPY_PROP":
            cfg = copy_propagation(cfg)

        if enabled_optimizations contains "DEAD_STORE_ELIM":
            cfg = dead_store_elimination(cfg)

      ❸ optimized_function_body = cfg_to_instructions(cfg)

      ❹ if (optimized_function_body == function_body
            or optimized_function_body is empty):
            return optimized_function_body

        function_body = optimized_function_body
```

*清单 19-6：TACKY 优化流水线*

在这个清单中，function_body 是 TACKY 函数体中的指令列表，enabled_optimizations 是一个字符串列表，表示我们在命令行上启用了哪些优化。（在真实程序中，这样存储命令行选项相当笨拙；在你自己的代码中完全可以用别的方式表示这些选项。）如果 function_body 为空，直接返回即可，因为没什么可优化的。否则，若启用了常量折叠，就执行它 ❶。

接下来，我们把函数体从指令列表转换为控制流图 ❷，并对这种表示应用其他所有已启用的优化。然后，把优化后的控制流图转换回指令列表 ❸，再与原始列表比较 ❹。如果两者不同，而且我们还没有把整个函数优化掉，就再循环一遍，以利用任何新的优化机会。如果相同，说明无法进一步优化，就此收工。

**我们怎么知道清单 19-6 会终止？**

乍一看，清单 19-6 中的 optimize 函数似乎可能陷入死循环。被优化的 TACKY 函数可能在每一轮迭代中都不断变化，永远收敛不到最终结果。但只要更仔细地思考这些优化，我们就能说服自己：优化流水线一定会终止。

首先，死存储消除和不可达代码消除都只会删除指令，而我们的优化从不新增指令。这两种优化不可能永远改变一个函数，因为指令迟早会被删完。更精确地说，如果 TACKY 函数最初有 n 条指令，那么这两种优化中有任何一个带来变化的循环迭代至多出现 n 次。

类似地，常量折叠会把几种 TACKY 指令（包括 Unary、Binary、类型转换和条件跳转）替换成其他种类的 TACKY 指令（具体是 Copy 和无条件 Jump 指令）。而其他优化都不会引入常量折叠所要替换的那些种类的指令。常量折叠能改变函数的次数是有限的，最终它可能改写的每一条指令都会被改写完。

剩下的就是复制传播。其他每种优化能改变函数的次数都已有了上界，所以只要复制传播本身不会陷入每次应用都改变函数的死循环，optimize 就会终止。为了说明这不可能发生，我们来想想复制传播对单条指令 i 最多能改写多少次。假设这条指令只有一个操作数，不过这个论证很容易推广到多操作数的指令。如果通往 i 的最短路径上没有 Copy 指令，那么 i 永远不会被改写。（记住，只有当一条 Copy 指令出现在通往 i 的每条路径上时，我们才能传播它。）现在假设通往 i 的路径上恰好有一条 Copy。这种情况下，i 至多能被改写一次。我们也许会把那条 Copy 指令的值传播给 i，但之后再也无法改写它了。再推广一下：如果通往 i 的最短路径上有 n 条 Copy 指令，那么 i 至多被改写 n 次。每当我们改写 i，都会把它的操作数替换为最短路径上定义得更早的另一个操作数（或一个常量）。如果 i 被改写多次，每次改写所传播的 Copy 指令在程序中的位置都必须比上一次更早，直到再没有可传播的复制为止。

看一个例子会更有帮助：

```text
w = foo()
x = w
y = x
Return(y)
```

最初，这段代码返回的是 y，它由 y = x 定义。一轮复制传播后，我们把最后一条指令改写为 Return(x)。当然，x 比 y 定义得更早，是在指令 x = w 中。下一轮复制传播会把 x 替换为 w，而 w 定义得更早，在指令 w = foo() 中。这里的关键在于，我们永远不会把一个操作数替换为通往被改写指令的路径上定义得更晚的另一个操作数，因此每个操作数只能被替换有限次。所以，optimize 函数必然终止。

使用清单 19-6 的优化流水线，我们绝不会漏掉任何优化机会。只要某个优化改变了程序，我们就重新运行其他三个，利用这些变化。这之所以可行，是因为我们只实现四种优化，而且所有测试程序都足够小，优化起来很快。生产级编译器实现了几十种优化、编译的程序也大得多，它们不采用这种方式；否则编译时间会长得离谱。它们只按固定顺序把各个优化跑一遍，并在最可能见效的位置运行每个单独的优化。因此，它们最终可能漏掉一些优化机会。（为给定程序寻找运行优化的最佳顺序是一个开放的研究问题，称为阶段排序问题（phase ordering problem）。）

动手把优化流水线加进你的编译器吧。目前，先把每个单独的优化定义为桩（stub）：接受指令列表并原样返回。到控制流图和从控制流图回来的转换也可以同样先写成桩。现在就把这些管线代码写好，这样实现各个优化遍时就能随时测试。

一切都接好之后，你就可以开始第一个优化了：常量折叠！

## 常量折叠

常量折叠是本章最简单的优化。这一遍遍历 TACKY 函数中的所有指令，并对源操作数为常量的指令求值。我们首先简单谈谈如何把常量折叠加入你在第一部分实现的编译器版本，然后讨论如何处理你在第二部分添加的类型和 TACKY 指令。如果你还没有做过第二部分，后面的讨论可以放心跳过。

### 第一部分 TACKY 程序的常量折叠

常量折叠遍应当对第一部分的四种 TACKY 指令求值：Unary、Binary、JumpIfZero 和 JumpIfNotZero。遇到源操作数为常量的 Unary 指令，或者两个源操作数都是常量的 Binary 指令时，把它替换为 Copy。例如，你应当把

```text
Binary(binary_operator=Add, src1=Constant(1), src2=Constant(2), dst=Var("b"))
```

替换为：

```text
Copy(src=Constant(3), dst=Var("b"))
```

常量折叠遍可能遇到两类非法表达式：除零和导致整数溢出的运算。两者都是未定义行为，所以怎么求值都无所谓。但编译器遇到这类非法表达式不能直接报错失败，因为只有在运行时真的执行到非法表达式，程序行为才是未定义的。例如，如果程序在一个永远不会走的分支里包含除零运算，你仍然应该能编译它。

还应当对条件为常量的 JumpIfZero 和 JumpIfNotZero 指令求值。如果条件满足，把指令替换为无条件 Jump；如果条件不满足，从程序中删除该指令。就这么简单！如果你只完成了第一部分，实现完这四种指令的常量折叠后，就可以直接跳到测试套件。如果你完成了第二部分，还有几种指令需要处理。

### 支持第二部分的 TACKY 程序

在第二部分添加新的算术类型时，我们还添加了类型转换指令：Truncate、SignExtend、ZeroExtend、DoubleToInt、DoubleToUInt、IntToDouble 和 UIntToDouble。当这些指令的源操作数是常量时，常量折叠遍应当对其求值。

Copy 指令也能执行类型转换；我们用它在同尺寸的有符号整数和无符号整数之间转换。当 Copy 指令把无符号常量复制到有符号变量（或相反）时，这一遍应当把常量转换为正确的类型。例如，若 a 是 signed char，你应当把

```text
Copy(src=Constant(ConstUChar(255)), dst=Var("a"))
```

替换为：

```text
Copy(src=Constant(ConstChar(-1)), dst=Var("a"))
```

注意，每次类型转换都要严格采用程序在运行时会使用的语义。例如，把 double 转换为整数类型时要向零截断；把整数转换为 double 时要舍入到最近的可表示值。好消息是，你已经知道如何在编译期执行所有这些类型转换，因为在整个第二部分中，你都需要把静态初始化器转换为正确的类型。理想情况下，你可以复用已写好的执行这些类型转换的代码。

求值无符号算术运算时，还需要遵循 C 的语义。特别是要确保无符号运算像运行时那样回绕（wrap around）。如何做到这一点，完全取决于你用什么语言编写编译器。有些语言在标准库中支持回绕无符号算术。例如在 Rust 中，wrapping_add 和 wrapping_sub 等方法提供与 C 中无符号算术相同的语义。在其他语言中，你可以用第三方库做无符号算术。例如，Python 没有无符号整数类型，但 NumPy 库有。如果不想用外部库，或者找不到合适的，自己实现回绕无符号算术也不算太难。

最后，求值浮点运算时，需要使用舍入到最近、平局取偶（round-to-nearest, ties-to-even）的舍入方式，并正确处理负零和无穷大。如果你在第 13 章的附加题中实现了对 NaN 的支持，还需要正确求值涉及 NaN 的运算。这不需要你付出特别的努力——绝大多数编程语言都采用 IEEE 754 语义——但你的实现语言在处理负零、NaN 或无穷大时与 C 有一点点不同的可能性是存在的。不妨先实现一个不处理这些边界情况的简单版常量折叠，依赖测试套件来捕捉问题。如果遇到实现语言求值不正确的情况，你有两个选择：要么找第三方库帮你处理，要么自己把它作为特例来求值。

**测试常量折叠遍**

如果你完成了第一部分但没做第二部分，用下面的命令测试常量折叠实现：

```bash
$ ./test_compiler /path/to/your_compiler --chapter 19 --fold-constants
--int-only
```

这会运行 tests/chapter_19/constant_folding/int_only 中的测试，这些测试只使用第一部分的语言特性。它还会在启用常量折叠的情况下编译第一部分的所有测试程序；对这批早期测试程序不会检查汇编输出，但会运行它们以确认行为依然正确。

如果你完成了第一和第二部分，运行同一命令并去掉 --int-only 选项：

```bash
$ ./test_compiler /path/to/your_compiler --chapter 19 --fold-constants
```

这条命令既运行 tests/chapter_19/constant_folding/int_only 中第一部分专用的测试，也运行 tests/chapter_19/constant_folding/all_types 中的测试，后者覆盖 int 之外类型的常量折叠。它还会在启用常量折叠的情况下运行第一和第二部分的所有测试。一如既往，你可以加上相应的标志，验证常量折叠遍能正确处理 NaN、位运算等附加题特性。

在实现复制传播之前，你的编译器还无法完全求值包含多个运算的嵌套源码级表达式（比如 -1 * 2），因为它无法把一个运算的结果传播给下一个运算。这意味着测试套件暂时覆盖不了某些情况，包括带负数或字符类型常量的常量折叠。本章末尾要运行的整条流水线测试会覆盖这些情况。在此期间，你可能想自己写一些单元测试来填补这些空缺。

## 控制流图

本章余下部分，我们将用控制流图来表示 TACKY 函数。对剩下的优化来说，图结构表示非常合适，因为它们必须考虑我们在函数中可能走的不同路径。控制流图中的节点表示称为基本块（basic block）的直线代码序列，另外有两个特殊节点分别表示函数的入口点和出口点。每个节点都有指向可能紧随其后执行的节点的出边。

举个例子，来看看清单 19-7 的控制流图。

```text
processing_loop():
    LoopStart:
    input = get_input()
    JumpIfNotZero(input, ProcessIt)
    Return(-1)
    ProcessIt:
    done = process_input(input)
    JumpIfNotZero(done, LoopStart)
    Return(0)
```

*清单 19-7：一个有多条执行路径的 TACKY 函数*

这个函数执行一个循环：反复调用 get_input 取得一个值，然后调用 process_input 处理该值。如果 get_input 返回 0，函数立即返回 -1；如果 process_input 返回 0，函数立即返回 0。图 19-1 给出了对应的控制流图。

*图 19-1：清单 19-7 的控制流图*

从特殊的 ENTRY 节点到块 B0 只有一条出边，因为函数开始时我们总会执行 B0。（在每个控制流图中，ENTRY 都恰好只有一条出边，因为 C 函数只有一个入口点。）执行完 B0 之后有两种可能：要么执行程序中的下一个块 B1，要么跳转到块 B2。因此 B0 到这两个块都有出边。同理，B2 到 B0 和 B3 都有出边。Return 指令会退出函数，所以 B1 和 B3 各有一条指向 EXIT 的出边。

### 定义控制流图

知道控制流图长什么样之后，来看看如何构造它。首先定义图的数据结构。清单 19-8 勾勒了一种可行的表示。

```text
node_id = ENTRY | EXIT | BlockId(int num)
node = BasicBlock(node_id id, instruction* instructions,
                  node_id* predecessors, node_id* successors)
     | EntryNode(node_id* successors)
     | ExitNode(node_id* predecessors)
graph = Graph(node* nodes)
```

*清单 19-8：控制流图的一种表示方法*

图中每个节点都有唯一的 node_id，标明它是 ENTRY、EXIT 还是一个编号的基本块。我们会按照基本块在原始 TACKY 函数中的顺序为其分配数字 ID。每个基本块保存一份 TACKY 指令列表、一个后继（successor，可能紧随其后执行的块）列表和一个前驱（predecessor，可能紧挨在其前执行的块）列表。入口节点和出口节点不保存任何指令。ENTRY 是函数中最早的点，有后继但没有前驱；EXIT 则有前驱但没有后继。

你还需要一种把额外信息关联到基本块和单条指令上的办法，以便在复制传播和死存储消除遍中追踪数据流分析的结果。清单 19-8 的定义没有包含追踪这些信息的手段。你可以把这些信息直接附在图上，也可以存放在独立的数据结构中。本章的伪代码将使用 annotate_instruction 和 get_instruction_annotation 来保存和查询关于单条指令的信息，使用 annotate_block 和 get_block_annotation 来按块 ID 保存和查询关于基本块的信息。

**注意**

你的图数据结构可能与清单 19-8 大相径庭。例如，你可以用从 node_id 到 node 的映射（而不是节点列表）来表示图，或者把入口/出口节点与表示基本块的节点分开管理。只要包含所需的全部信息，你可以按任何对你和你的实现语言来说合理的方式来定义控制流图。

### 创建基本块

接下来看看如何把 TACKY 函数体划分成基本块。基本块的中间不能有跳转进入或跳出。执行一个基本块的唯一方式是从它的第一条指令开始，一路执行到最后一条。这意味着 Label 只能作为块的第一条指令出现，而 Return 或跳转指令只能作为块的最后一条指令出现。清单 19-9 演示了如何沿这些边界把指令列表切分成基本块。

```text
partition_into_basic_blocks(instructions):
    finished_blocks = []
    current_block = []
    for instruction in instructions:
      ❶ if instruction is Label:
            if current_block is not empty:
                finished_blocks.append(current_block)
            current_block = [instruction]

      ❷ else if instruction is Jump, JumpIfZero, JumpIfNotZero, or Return:
            current_block.append(instruction)
            finished_blocks.append(current_block)
            current_block = []

        else:
          ❸ current_block.append(instruction)

    if current_block is not empty:
        finished_blocks.append(current_block)

    return finished_blocks
```

*清单 19-9：把指令列表划分成基本块*

遇到 Label 指令时，我们开始一个新的基本块，以该 Label 开头 ❶。遇到 Return 指令或条件/无条件跳转时，把它加入当前块，然后开始一个新的空块 ❷。遇到其他任何指令时，把它加入当前块而不另起新块 ❸。

清单 19-9 只是把函数体划分成一个由指令列表组成的列表。下一步（我不提供伪代码）是把这些指令列表转换成块 ID 递增的 BasicBlock 节点，然后把这些节点连同入口、出口节点一起加入图中。

### 为控制流图添加边

把所有节点加入图后，我们再从每个节点向它的后继添加边，如清单 19-10 所示。

```text
add_all_edges( ❶ graph):

  ❷ add_edge(ENTRY, BlockId(0))

    for node in graph.nodes:
        if node is EntryNode or ExitNode:
            continue

        if node.id == max_block_id(graph.nodes):
            next_id = EXIT
        else:
          ❸ next_id = BlockId(node.id.num + 1)

        instr = get_last(node.instructions)
        match instr with
        | Return(maybe_val) -> add_edge(node.id, EXIT)
        | Jump(target) ->
            target_id = get_block_by_label(target)
            add_edge(node.id, target_id)
        | JumpIfZero(condition, target) ->
            target_id = get_block_by_label(target)
          ❹ add_edge(node.id, target_id)
          ❺ add_edge(node.id, next_id)
        | JumpIfNotZero(condition, target) ->
            // 与 JumpIfZero 相同
            --snip--
        | _ -> add_edge(node.id, next_id)
```

*清单 19-10：为控制流图添加边*

add_all_edges 的参数 graph ❶ 是我们尚未完成的控制流图——有节点但没有边。我们首先从 ENTRY 向第一个基本块添加一条边 ❷。（可以假设函数至少包含一个基本块，因为我们不优化空函数。）在整个清单中，我们使用接受两个节点 ID 的 add_edge 函数向图中加边。记住，每当我们添加从 node1 到 node2 的边时，必须同时更新 node1 的后继列表和 node2 的前驱列表。add_edge 的伪代码我省略了，因为它取决于你如何定义控制流图。

接下来，我们为对应基本块的节点添加出边。处理这样一个节点时，首先要确定：如果块末尾既不跳转也不返回，默认接下来会执行哪个节点。如果正在处理的是最后一个块，下一个节点是 EXIT；否则就是原始 TACKY 函数中紧随其后的那个基本块 ❸。

加哪些边，要通过检查当前基本块的最后一条指令来确定。如果是 Return 指令，就添加一条指向 EXIT 的出边。如果是无条件 Jump，就向以对应 Label 开头的块添加一条边。我们用辅助函数 get_block_by_label（其伪代码我也不展示）来查询哪个块以特定标签开头。我建议提前构建一个从标签到块 ID 的映射，这样这个函数只需查一次映射。

如果块以条件跳转结尾，我们添加两条出边。第一条代表走跳转，指向以对应 Label 开头的块 ❹；另一条代表不走跳转 ❺，指向由 next_id 标识的默认下一节点。如果块以其他任何指令结尾，我们只添加一条指向默认下一节点的出边。

### 把控制流图转换回指令列表

到这里，你应该已经有了能把 TACKY 函数转换为控制流图的可用代码。你还需要反方向的代码，把控制流图转换回指令列表。这个操作简单得多：把所有基本块按 ID 排序，然后把它们的指令全部拼接起来即可。

### 让控制流图代码可复用

下一章我们将为汇编程序构建控制流图。构造这些图的算法相同，但要寻找的具体控制流指令不同。例如在汇编中，jmp、ret 以及 jne、je 等条件跳转指令都标志着一个基本块的结束。

一旦有了构造控制流图的可用代码，你可能会想重构它，使其也能用于汇编程序。这完全是可选的，但能为下一章省点力气。

首先，你需要泛化图的数据类型，使块中既可以装 TACKY 指令也可以装汇编指令。其次，你需要泛化清单 19-9 和 19-10 中分析具体指令的逻辑。例如，你可以定义一个一次性数据类型来同时表示汇编指令和 TACKY 指令，只捕获构建控制流图所需的信息：

```text
generic_instruction = Return
                    | Jump
                    | ConditionalJump(identifier label)
                    | Label(identifier)
                    | Other
```

这样一来，就不必通过检查单条 TACKY 指令来判断基本块在哪里结束、后继是谁，而是把每条指令转换为 generic_instruction 再检查它。之后需要为汇编程序构建控制流图时，只需换一个不同的辅助函数把汇编指令转换为 generic_instruction，其余一切保持不变。

关于控制流图的讨论到此结束。我们现在可以进入第二个优化遍：不可达代码消除。

## 不可达代码消除

我们把这一遍分成三步：先删除永远不会执行的基本块，再删除无用跳转，最后删除无用标签。后两步可能留下不含任何指令的空块。可选地，我们可以在这一优化结束后做点清理，把这些空块从控制流图中删掉。

### 消除不可达基本块

为了找出所有可能执行到的块，我们从 ENTRY 开始遍历控制流图。先访问 ENTRY 的后继，再访问该节点的所有后继，以此类推，直到没有节点可探索为止。如果遍历始终没有到达某个基本块，我们就知道这个块可以安全删除。在最初介绍不可达代码消除时看过的清单 19-4 的例子上试试这个方法：

```text
x = 5
Jump(Target)
x = my_function()
Target:
Return(x)
```

我们之前已经判定 x = my_function() 不可达。假设这个清单就是某个 TACKY 函数的完整函数体，它的控制流图如图 19-2 所示。

*图 19-2：清单 19-4 的控制流图*

注意，不存在从 ENTRY 到 B1 的路径。从 ENTRY 开始遍历这张图，我们会访问 B0、B2 和 EXIT。遍历过程中，我们记录到目前为止访问过哪些节点。遍历结束后，我们发现从未访问过 B1，于是把它删除。探索图的伪代码我就不给了，因为它不过是普通的广度或深度优先图遍历。

从图中删除节点时，记得把它的出边也删掉。例如，当我们从图 19-2 中删除 B1 时，还应当把它从 B2 的前驱列表中移除。

### 删除无用跳转

接下来删除无用的跳转指令。记住，默认情况下，如果一个块不以跳转或 Return 指令结尾，控制权会按原始程序顺序落入下一个块。如果一条跳转指令的目标正是这个默认的下一个块，就可以删掉它。

我们逐一考察以条件或无条件跳转结尾的基本块，确定如果不走跳转，默认接下来会是哪个块。如果这个默认的下一个块是它唯一的后继，这条跳转指令就是多余的。清单 19-11 演示了这个方法。

```text
remove_redundant_jumps(graph):
  ❶ sorted_blocks = sort_basic_blocks(graph)
    i = 0
  ❷ while i < length(sorted_blocks) - 1:
        block = sorted_blocks[i]
        if block.instructions ends with Jump, JumpIfZero, or JumpIfNotZero:
            keep_jump = False
            default_succ = sorted_blocks[i + 1]
            for succ_id in block.successors:
                if succ_id != default_succ.id:
                    keep_jump = True
                    break
            if not keep_jump:
              ❸ remove_last(block.instructions)
        i += 1
```

*清单 19-11：删除多余的跳转*

首先，按基本块在原始 TACKY 函数中的位置排序 ❶——这也是我们当初构造图时给块编号的原因之一。然后遍历这个有序的基本块列表（最后一个块除外，因为函数最末尾的跳转永远不会是多余的）❷。如果一个块以跳转结尾，我们就在它的后继中寻找列表中下一个块之外的后继。找到就保留跳转指令，否则删除它 ❸。

注意，列表中的下一个块，其数字 ID 未必连续，因为我们可能已经删除了一些块。比如块 2 后面跟的可能是块 4。这就是为什么不能简单地把块的 ID 加 1 来找它的默认后继。

### 删除无用标签

删除无用标签与删除无用跳转类似。把基本块按数字 ID 排序后，如果进入某个块只会经由前一个块直接落入、而不是显式跳转过来，就可以删除该块开头的 Label 指令。更具体地说，如果 sorted_blocks[i] 的唯一前驱是 sorted_blocks[i - 1]，就可以删除它开头的 Label；如果 sorted_blocks[0] 的唯一前驱是 ENTRY，也可以删除它开头的 Label。这个变换是安全的，因为我们刚刚删除了多余的跳转指令；我们知道 sorted_blocks[i - 1] 不会以一条显式跳转到 sorted_blocks[i] 的指令结尾。这一步的伪代码我就不给了，因为它和清单 19-11 基本一样。

### 删除空基本块

删除不可达跳转和标签之后，可能产生没有任何指令的块。愿意的话可以把它们删掉；这能缩小图的规模，也许还能让后续的优化遍快一点。删除块时，务必相应地更新控制流图中的边。例如，如果图中有 B0 到 B1 的边和 B1 到 B2 的边，删除 B1 之后就需要添加一条从 B0 到 B2 的边。

**测试不可达代码消除遍**

如果你只完成了第一部分，用下面的命令测试这个优化遍：

```bash
$ ./test_compiler /path/to/your_compiler --chapter 19 --eliminate-unreachable-code
--int-only
```

这会运行 tests/chapter_19/unreachable_code_elimination 中的测试。其中一些测试依赖于你本章前面实现的常量折叠遍。它们都不使用第二部分添加的特性，因为那些特性与这个优化没有交互。这条命令还会在同时启用常量折叠和不可达代码消除的情况下重跑第一部分的测试。若要连第二部分的测试也重跑，去掉 --int-only 选项：

```bash
$ ./test_compiler /path/to/your_compiler --chapter 19 --eliminate-unreachable-code
```

## 数据流分析简介

本节将快速概述数据流分析，后面两个优化遍都要依赖它。你将了解它是什么、何时有用，以及所有数据流分析共有的特征。这并不是对数据流分析的完整讲解；这里的目标只是介绍几个关键概念并说明它们如何组合在一起，让后面几节的具体分析更容易理解。

数据流分析回答的是关于值在整个函数中如何被定义和使用的问题。不同的数据流分析回答不同的问题。例如，在复制传播遍中，我们将实现到达复制分析（reaching copies analysis）。它回答的问题是：给定 TACKY 函数中的某条指令 i，以及该函数中出现的两个操作数 u 和 v，我们能否保证在 i 执行之前的那一刻 u 和 v 相等？

所有数据流分析都可以分为两大类：前向（forward）分析和反向（backward）分析。在前向分析中，信息沿控制流图向前传播。到达复制分析是前向分析。看到 Copy 指令 x = y 时，我们知道在同一基本块的后面或在该块某个后继中，x 和 y 可能具有相同的值。反向分析则相反。在死存储消除遍中，我们将实现一种叫活性分析（liveness analysis）的反向分析。这种分析告诉我们变量的当前值日后是否会被使用。如果看到一条使用 x 的指令，说明在同一基本块的更早位置或该块某个前驱中，x 可能是活的（live）。

每种数据流分析都有自己的传递函数（transfer function）和汇聚算子（meet operator）。传递函数计算单个基本块内部的分析结果。它分析单条指令如何影响结果，但不需要处理多条执行路径。汇聚算子合并来自多条路径的信息，计算每个基本块如何受其相邻块影响。我们将用迭代算法（iterative algorithm）驱动整个分析。这个算法在每个基本块上调用传递函数和汇聚算子，并记录哪些块还需要分析。说它是迭代的，是因为在沿不同执行路径传播信息的过程中，某些块可能需要访问多次。该算法会遍历控制流图，分析访问到的每个基本块，直到达到分析结果不再变化的不动点。到那时，我们就知道每条可能的执行路径都已被考虑在内。迭代算法并不是求解数据流分析问题的唯一方法，但它是本书讨论的唯一方法。

虽然不同的分析使用不同的传递函数和汇聚算子，但它们使用的迭代算法本质上相同。前向和反向分析传播数据的方向相反，因此使用这个算法的不同版本。接下来两节我们会把两个版本都实现一遍。

## 复制传播

如果函数中出现指令 x = y，我们有时可以在函数后面的位置用 y 替换 x。把我们想执行替换的那条指令称为 i。替换在两个条件都满足时才是安全的。第一，x = y 必须出现在从程序入口点到 i 的每一条路径上。看图 19-3 中的控制流图，它不满足这个条件。

*图 19-3：一个无法执行复制传播的函数的控制流图*

在这张控制流图中，从函数开头到 Return(x) 有两条路径。由于只有一条经过 x = 2，在这条 Return 指令中把 x 替换为 2 并不安全。而图 19-4 中，通往 Return(x) 的每条路径都经过 x = 2。

*图 19-4：一个可以执行复制传播的函数的控制流图*

无论走图 19-4 中的哪条路径，我们都会在到达 Return 指令之前执行 x = 2，所以可以安全地把那条指令改写为 Return(2)。

图 19-5 给出了另一个稍微棘手的例子。

*图 19-5：另一个可以安全执行复制传播的控制流图*

这里同样有两条通往 Return(x) 的不同路径。两条路径都经过 x = y，但经过的是出现在不同块中的不同实例。在 B1 中 y 的值是 20，在 B2 中是 100。但无论哪种情况，到达 Return 指令时 x 和 y 都具有相同的值。这意味着把 Return(x) 改写为 Return(y) 依然是安全的。

在改写指令 i 之前，通往 i 的每条路径还必须满足第二个条件：在指令 x = y 与 i 之间，x 和 y 都不能再被更新。考虑这段 TACKY：

```text
x = 10
x = foo()
Return(x)
```

我们不能在 Return(x) 中把 x 替换为 10，因为在那一时刻 x 的值已不再是 10。更新出现在 Copy 指令右侧的变量也会导致同样的问题：

```text
x = y
y = 0
Return(x)
```

在 y = 0 之前，我们知道 x 和 y 的值相同；但这条指令执行之后，两者的值就不同了，所以不能改写 Return(x)。当 Copy 指令的源或目的地被更新时，我们说这个复制被杀死（kill）了。复制一旦被杀死，就不能再传播到程序中更靠后的位置。

在通往 i 的某条路径上，x = y 可能出现多次。只有当它在最后一次出现之后被杀死，传播才是不安全的。在下面的例子中，把 Return(x) 改写为 Return(2) 是安全的：

```text
x = 2
x = foo()
x = 2
Return(x)
```

如果通往 i 的路径有多条，我们关心的那条 Copy 指令在任何一条路径上都不能被杀死。看看图 19-6，其中 x = y 在一条路径上被杀死，在另一条上没有。

*图 19-6：一个到达复制在某条路径上被杀死的控制流图*

如果跳过 B1，到达 B2 中的 Return 指令时 x 和 y 的值相同。但如果走经过 B1 的路径，两者的值就不同了。由于我们无法预先知道程序会走哪条路径，所以不能改写 Return(x)。

再考虑最后一种边界情况。假设 x = y 后面跟着 y = x，中间没有任何杀死操作：

```text
x = y
--snip--
y = x
z = x + y
```

按理说，更新 y 会杀死前面那条 Copy 指令。但在 y = x 之后，x 和 y 的值仍然相同。处理这种情况有多种正确的办法。一种办法是认定 y = x 杀死了 x = y，于是只有 y = x 到达 z = x + y。这种情况下，我们会把最后的指令改写为 z = x + x。取决于 y 在别处还有没有使用，这可能让我们稍后在死存储消除中删掉 y = x。另一种办法是在分析中干脆忽略 y = x，理由是它不产生任何效果；它只是给 y 赋了它已有的值。然后在改写指令时，我们可以顺势消除 y = x，并把最后的指令改写为 z = y + y。第三种办法是在最后的指令中同时传播两个复制，把 y 换成 x、把 x 换成 y。这种替换是安全的，但没什么帮助，因为它无助于摆脱任何一条 Copy 指令。我们采用第二种办法，消除多余的 Copy。

如果一条 Copy 指令出现在通往指令 i 的每条路径上，并且在任何一条路径上都没有被杀死，我们就说它到达（reach）指令 i。复制传播遍开始时，我们先执行到达复制分析，确定哪些复制到达 TACKY 函数中的每条指令，然后利用分析结果找出可以安全改写的指令。

我们先针对第一部分定义的 TACKY 子集实现整个优化，然后把它扩展到第二部分的新语言特性。

### 到达复制分析

实现到达复制分析，需要定义我们之前讨论过的数据流分析的各个要素：传递函数、汇聚算子和迭代算法。本节讨论的传递函数和汇聚算子是到达复制分析专用的，而迭代算法适用于每一种前向数据流分析。

#### 传递函数

传递函数接受到达基本块开头的所有 Copy 指令，计算哪些复制到达块内的每条指令。它还计算哪些复制到达块的末尾，即最后一条指令之后。规则很简单。第一，如果 i 是 Copy 指令，它到达紧随其后的指令。第二，如果某个复制到达 i，它也到达 i 的下一条指令，除非 i 杀死了它。来做一个例子。假设某个基本块包含清单 19-12 中的指令。

```text
x = a
y = 10
x = y * 3
Return(x)
```

*清单 19-12：一个基本块*

假设有一条 Copy 指令 a = y 到达这个基本块的开头。这条 Copy 会到达第一条指令 x = a。遇到 x = a 后，我们把它加入当前的到达复制集合，于是 a = y 和 x = a 都到达下一条指令 y = 10。由于这下一指令更新了 y，它杀死了 a = y。因此我们从到达复制集合中删掉 a = y，但加入 y = 10。最后，x = y * 3 杀死了 x = a。我们不把 x = y * 3 加入到达复制，因为它不是 Copy 指令。末尾的 Return 指令既不增加也不删除任何到达复制。表 19-1 列出了到达这个基本块中每条指令的复制。

| 指令 | 到达复制 |
| --- | --- |
| x = a | {a = y} |
| y = 10 | {a = y, x = a} |
| x = y * 3 | {x = a, y = 10} |
| Return(x) | {y = 10} |
| 块末尾 | {y = 10} |

*表 19-1：到达清单 19-12 中各指令的复制*

一旦考虑静态存储期的变量，事情就变得更棘手了。如清单 19-13 所示，这些变量可以在其他函数中被更新。

```c
int static_var = 0;

int update_var(void) {
    static_var = 4;
    return 0;
}

int main(void) {
    static_var = 5;
  ❶ update_var();
    return static_var;
}
```

*清单 19-13：多个函数访问同一个静态存储期变量的 C 程序*

到达复制分析应当识别出 main 中对 update_var 的调用杀死了 static_var = 5 ❶。否则，它会错误地把 main 改写成返回常量 5。乍一看，这个问题似乎只适用于文件作用域变量，但正如清单 19-14 所示，它同样影响静态局部变量。

```c
int indirect_update(void);

int f(int new_total) {
    static int total = 0;
    total = new_total;
    if (total > 100)
        return 0;
    total = 10;
  ❶ indirect_update();
    return total;
}

int indirect_update(void) {
    f(101);
    return 0;
}
```

*清单 19-14：函数调用间接更新静态局部变量的 C 程序*

分析 f 时，我们必须知道 ❶ 处对 indirect_update 的调用可能更新 total。否则我们会错误地把 f 改写成返回 10。

解决这个问题有几种办法。一种是搞清楚哪些函数调用会更新哪些静态变量。这会把到达复制分析变成过程间分析（interprocedural analysis），即收集多个函数信息的分析。这种办法很快就会变得非常复杂。另一种办法是假设每个函数调用都会更新每个静态变量。我们选择后者，因为它简单得多。每当遇到函数调用，我们就杀死所有与静态变量相关的复制。这是一种保守的做法；它保证我们绝不会执行不安全的优化，但可能导致我们不必要地杀死一些到达复制，错过一些安全的优化。相比之下，使用过程间分析是更激进的做法，因为它漏掉的优化更少。但更激进的优化技术并不总是更好；它们往往以更高的复杂度和更长的编译时间为代价。

清单 19-15 给出传递函数的伪代码。

```text
transfer(block, initial_reaching_copies):
    current_reaching_copies = initial_reaching_copies

    for instruction in block.instructions:
      ❶ annotate_instruction(instruction, current_reaching_copies)
        match instruction with
        | Copy(src, dst) ->
          ❷ if Copy(dst, src) is in current_reaching_copies:
                continue

          ❸ for copy in current_reaching_copies:
                if copy.src == dst or copy.dst == dst:
                    current_reaching_copies.remove(copy)

          ❹ current_reaching_copies.add(instruction)
        | FunCall(fun_name, args, dst) ->
            for copy in current_reaching_copies:
              ❺ if (copy.src is static
                    or copy.dst is static
                    or copy.src == dst
                    or copy.dst == dst):
                    current_reaching_copies.remove(copy)
        | Unary(operator, src, dst) ->
          ❻ for copy in current_reaching_copies:
                if copy.src == dst or copy.dst == dst:
                    current_reaching_copies.remove(copy)
        | Binary(operator, src1, src2, dst) ->
            // 与 Unary 相同
            --snip--
        | _ -> continue

  ❼ annotate_block(block.id, current_reaching_copies)
```

*清单 19-15：到达复制分析的传递函数*

处理一条指令时，我们首先记录到达该指令执行之前那一刻的复制集合 ❶（稍后实际改写指令时会用到这个信息），然后检查指令本身，计算哪些复制到达它之后的位置。在 x = y 到达 y = x 这个特例中，我们不增不减任何到达复制 ❷。如前所见，y = x 不会产生效果，因为 x 和 y 已经具有相同的值。否则，处理 Copy 指令 x = y 的方式是：先杀死所有与 x 相关的复制 ❸，再把 x = y 加入到达复制集合 ❹。

遇到 FunCall 指令时，我们杀死所有与静态存储期变量相关的复制，以及所有与 dst（将保存函数调用结果的变量）相关的复制 ❺。第一部分中另外两种会更新变量的指令是 Unary 和 Binary。处理它们时，杀死所有与其目的地相关的复制 ❻。第一部分其余的 TACKY 指令（如 Jump、Return 和 Label）既不增加也不杀死到达复制。处理完所有指令后，我们记录哪些复制到达块的末尾 ❼。

#### 汇聚算子

接下来实现汇聚算子，它把到达复制的信息从一个块传播到另一个块。这个算子计算我们将传给传递函数的初始到达复制集合。回想一下，一个复制只有在通往某点的每条路径上都出现且未被杀死时，才到达该点。因此，一个复制只有在到达某块所有前驱的末尾时，才到达该块的开头。换句话说，我们只需对每个前驱的结果取集合交集。清单 19-16 给出汇聚算子的伪代码。

```text
meet(block, all_copies):

  ❶ incoming_copies = all_copies
    for pred_id in block.predecessors:
        match pred_id with
      ❷ | ENTRY -> return {}
        | BlockId(id) ->
          ❸ pred_out_copies = get_block_annotation(pred_id)
            incoming_copies = intersection(incoming_copies, pred_out_copies)
        | EXIT -> fail("Malformed control-flow graph")

    return incoming_copies
```

*清单 19-16：到达复制分析的汇聚算子*

汇聚算子接受两个参数。第一个是待计算传入复制的块；第二个 all_copies 是函数中出现的所有 Copy 指令的集合。我们把传入复制集合初始化为这个值 ❶，因为它是集合交集的单位元（identity element）。也就是说，对任何到达复制集合 S，S 与 all_copies 的交集就是 S 本身。

接下来遍历该块的前驱，其中可能包括其他基本块、ENTRY 节点或两者都有。没有任何复制会到达函数的最开头，因此如果在前驱列表中发现 ENTRY，就直接返回空集 ❷。（空集与任何集合的交集仍是空集，所以不必再看该块的其他前驱。）否则，查询到达每个前驱末尾的复制集合 ❸（我们在清单 19-15 的末尾记录过），并让 incoming_copies 与这些集合逐个取交集。

还有一种边界情况要考虑。如果禁用了不可达代码消除，被分析的块可能没有任何前驱。对没有前驱的块调用 meet 会返回 all_copies，等于假设每个可能的 Copy 指令都到达该块的开头。这个块反正永远不会执行，所以它本身受什么影响我们并不在意；我们在意的是这会如何影响该块的后继，因为后继可能是可达的。例如，可达块 A 和不可达块 B 都跳转到块 C，那么块 C 是可达的。

幸运的是，我们的分析依然是安全的。来自 A 的真实结果与来自 B 的垃圾结果的交集，永远是实际从 A 到达 C 的复制的子集；这相当于启用不可达代码消除、彻底删除 B 后所得结果的一个保守近似。

#### 迭代算法

一旦知道了前驱块的结果，我们就可以用汇聚算子和传递函数分析一个基本块。现在把一切都串起来，分析整个函数。只有一个问题：控制流图可能有循环！在分析完一个块的所有前驱之前，我们无法分析这个块；而分析那些前驱又要求先分析它们的前驱，依此类推。一旦碰到循环，我们似乎就被卡住了：循环中的任何块都无法分析，因为每个块都直接或间接地是它自己的前驱。

要走出僵局，我们需要一种即便没有某块全部前驱的完整结果也能分析该块的办法。解决方案是为每个块维护一个临时结果；如果不得不在部分前驱尚未分析完之前分析一个块，就使用那些前驱的临时结果。最初，在还没有探索任何通往该块的路径时，它的临时结果包含函数中的每一条 Copy 指令。随后，每探索一条通往该块（确切说是该块末尾）的新路径，我们就剔除那些没有在这条路径上出现、或在这条路径上被杀死的复制。这意味着块的临时结果始终告诉我们：在目前已探索的通往该块末尾的每条路径上，哪些到达复制会出现（且未被杀死）。一旦探索完所有可能的路径，我们就得到了该块的最终结果。

基本思路就是这些；现在付诸实践。首先，给每个基本块标注上函数中所有 Copy 指令的集合。如前所述，这个集合是我们汇聚算子的单位元。用单位元初始化每个块，可以保证尚未分析的块不会改变汇聚算子的结果。在图 19-7 的控制流图上试试这个方法。

*图 19-7：一个带循环的控制流图*

这张控制流图包含两条 Copy 指令：y = 3 和 y = 4。我们最初给每个块标注包含这两个复制的集合，然后按顺序分析各块。B0 的唯一前驱是 ENTRY，所以一遍就能算出它的最终结果。图 19-8 展示了处理完 B0 之后各块上的标注。

*图 19-8：图 19-7 的到达复制分析的临时结果（处理完 B0 之后）*

此时 B0 上的标注是正确的：只有 y = 3 到达该块末尾。另外两个块仍标注着全部复制。接下来应用汇聚算子，看看哪些复制到达 B1 的开头。这个块有两个前驱：B0 和它自己。于是我们对 {y = 3} 和 {y = 3, y = 4} 取交集，得到 {y = 3}。这与 B0 是 B1 唯一前驱时得到的结果相同。这正是我们想要的行为：因为还没有分析 B1，它不应对汇聚算子的结果产生影响。对 B1 应用传递函数后，我们会发现只有 y = 4 到达该块末尾。这样，处理 B2 所需的信息也就齐了。图 19-9 展示了分析完 B1 和 B2 之后各块上的标注。

*图 19-9：每个基本块各分析一次后，到达复制分析的临时结果*

现在每个块都有了正确的到达复制集合。但对 B1 中每条具体指令而言，答案还不对。（图 19-8 和图 19-9 没有展示单条指令上的标注。）上次分析 B1 时，我们假设 y = 3 到达该块开头，这意味着它也到达 x = process(y)。既然现在有了更准确的信息，就需要再分析一次 B1。这一次，汇聚算子会对 {y = 3} 和 {y = 4} 取交集，得到空集。我们把这个结果传给传递函数，重新计算 B1 中各指令的结果。这一回，我们会正确地得出结论：没有任何 Copy 指令到达 x = process(y)（也不到达该块中 y = 4 之前的任何位置）。

见识了迭代算法的实际运行之后，我们来实现它。清单 19-17 给出这个算法的伪代码。

```text
find_reaching_copies(graph):

    all_copies = find_all_copy_instructions(graph)
    worklist = []

    for node in graph.nodes:
        if node is EntryNode or ExitNode:
            continue
      ❶ worklist.append(node)
        annotate_block(node.id, all_copies)

    while worklist is not empty:
      ❷ block = take_first(worklist)
        old_annotation = get_block_annotation(block.id)
        incoming_copies = meet(block, all_copies)
        transfer(block, incoming_copies)
      ❸ if old_annotation != get_block_annotation(block.id):
            for successor_id in block.successors:
                match successor_id with
                | EXIT -> continue
                | ENTRY -> fail("Malformed control-flow graph")
                | BlockId(id) ->
                    successor = get_block_by_id(successor_id)
                    if successor is not in worklist:
                        worklist.append(successor)
```

*清单 19-17：到达复制分析的迭代算法*

我们维护一个待处理基本块的工作列表（worklist），其中包括在前驱更新之后需要重新访问的块。算法的初始设置会把每个基本块加入工作列表 ❶，因为每个块至少要分析一次。我们还会用函数中所有 Copy 指令的集合初始化每个块。

接下来进入主处理循环：从工作列表头部取出一个块 ❷，用汇聚算子和传递函数分析它。如果分析改变了该块对外的到达复制集合，就把它的所有后继加入工作列表，以便用新结果重新分析它们 ❸。如果某个后继已在工作列表中，就不必重复添加。重复这个过程，直到工作列表为空。

**让基本块物尽其用**

清单 19-17 的代码还有改进空间：初始化工作列表时，可以按逆后序（reverse postorder）添加块。要把图的节点按后序（postorder）排序，需执行一次深度优先搜索，并在从某节点的后继遍历返回之后，才把该节点加入有序列表。要按逆后序排序，就把后序排好的节点列表反转过来。在逆后序遍历中，通常要等访问完一个块的所有前驱之后才会访问这个块。（当然，如果遍历的图里有循环，这条一般规则会有少数例外。）这有助于在分析一个块之前尽可能多地收集其前驱的信息，从而把每个块需要重新访问的次数降到最低。如果不按这种顺序把块加入工作列表，算法依然正确，只是耗时更久。第 610 页的「延伸资源」列出了几篇参考文献，详细介绍了如何把图按后序或逆后序排序。

清单 19-17 适用于任何前向数据流分析。会变化的只有传递函数、汇聚算子，以及用于初始化每个基本块的单位元。

### 改写 TACKY 指令

运行完到达复制分析后，我们寻找机会改写甚至删除 TACKY 函数中的每条指令。要改写一条指令，我们先检查到达它的复制是否定义了它的某个操作数；如果有，就把这些操作数替换为它们的值。如果遇到形如 x = y 的 Copy 指令，而它的到达复制中包含 x = y 或 y = x，我们就不改写它而是直接删除；因为如果 x 和 y 已经同值，这条指令不会产生任何效果。清单 19-18 演示了如何处理每条指令。

```text
replace_operand(op, reaching_copies):
    if op is a constant:
        return op

    for copy in reaching_copies:
        if copy.dst == op:
            return copy.src
    return op

rewrite_instruction(instr):
  ❶ reaching_copies = get_instruction_annotation(instr)
    match instr with
    | Copy(src, dst) ->
        for copy in reaching_copies:
          ❷ if (copy == instr) or (copy.src == dst and copy.dst == src):
                return null
      ❸ new_src = replace_operand(src, reaching_copies)
        return Copy(new_src, dst)
    | Unary(operator, src, dst) ->
        new_src = replace_operand(src, reaching_copies)
        return Unary(operator, new_src, dst)
    | Binary(operator, src1, src2, dst) ->
        new_src1 = replace_operand(src1, reaching_copies)
        new_src2 = replace_operand(src2, reaching_copies)
        return Binary(operator, new_src1, new_src2, dst)
    | --snip--
```

*清单 19-18：根据到达复制分析的结果改写指令*

给定到达当前指令的复制集合，replace_operand 用值替换单个 TACKY 操作数。如果操作数是常量，或者找不到给该操作数赋值的到达复制，就直接返回原值。

在 rewrite_instruction 中，我们首先查询到达当前指令 instr 的复制集合 ❶。如果 instr 是 Copy 指令，就在这个我们称为 reaching_copies 的集合中查找从它的源到目的地（或反过来）的复制 ❷。找到了，就说明 instr 的两个操作数已经同值，可以删除它。（清单 19-18 返回 null 表示应当删除该指令；你的代码可以用别的方式表达。）否则，用 replace_operand 尝试替换该指令的源操作数 ❸。其他 TACKY 指令的源操作数也照此替换。清单 19-18 演示了 Unary 和 Binary 的源操作数改写；第一部分其余的 TACKY 指令我省略了，因为逻辑相同。

到这里，你就有了一个完整的复制传播遍：执行到达复制分析，并利用结果优化 TACKY 函数。如果你跳过了第二部分，可以直接进入本节的测试套件。如果你完成了第二部分，还有一些工作要做。

### 支持第二部分的 TACKY 程序

要让复制传播处理我们在第二部分生成的 TACKY 代码，需要解决两个问题。第一个问题是，我们有时用 Copy 指令执行类型转换。我们不想在有符号类型与无符号类型之间传播复制，因为对有符号值和无符号值的运算，我们有时会生成不同的汇编代码。例如，如果在比较中把有符号值替换为无符号值，最终会为该比较生成错误的条件码（condition code）。我们的到达复制分析会把有符号与无符号操作数之间的任何 Copy 当作类型转换指令，而不是普通的复制操作。我们不会在传递函数中把它加入到达复制，也不会在迭代算法开始时把它放进初始到达复制集合。

**注意**

另一种解决办法是为比较、求余和除法引入分开的有符号与无符号 TACKY 运算符，这样代码生成时就无需通过检查操作数类型来区分这些情况。LLVM IR 采用的就是这种做法。

第二个问题是，变量可以通过指针被更新。这类更新很难分析。看到指令 Store(v, ptr) 时，我们不知道 ptr 指向哪个对象，因此不知道该杀死哪些复制。这与静态变量带来的问题类似——它们可能在其他函数中被更新。为了解决这个问题，我们要找出所有可能通过指针访问的变量（称为别名变量，aliased variable），并假设每条 Store 指令都会更新所有这些变量。我们同样假设函数调用也会更新这些变量，因为可以在一个函数中声明变量，然后在另一个函数中通过指针更新它。用这个方法来分析清单 19-19。

```text
function_with_pointers():
    x = 1
    y = 2
    z = 3
    ptr1 = GetAddress(x)
    Store(10, ptr1)
    ptr2 = GetAddress(y)
    z = x + y
    Return(z)
```

*清单 19-19：通过指针更新变量的 TACKY 函数*

首先找出 function_with_pointers 中的别名变量。x 和 y 都是别名变量，因为它们都出现在 GetAddress 指令中。（假设这个清单里的变量都不是静态的，这样就不必担心其他函数取它们的地址。）接着运行到达复制分析。由于整个函数体就是一个基本块，只需对它整体应用传递函数。我们照常把 x = 1、y = 2 和 z = 3 加入到达复制集合。然后，遇到 Store 指令时，杀死与两个别名变量 x 和 y 相关的复制。表 19-2 描述了到达该函数中每条指令的复制。

| 指令 | 到达复制 |
| --- | --- |
| x = 1 | {} |
| y = 2 | {x = 1} |
| z = 3 | {x = 1, y = 2} |
| ptr1 = GetAddress(x) | {x = 1, y = 2, z = 3} |
| Store(10, ptr1) | {x = 1, y = 2, z = 3} |
| ptr2 = GetAddress(y) | {z = 3} |
| z = x + y | {z = 3} |
| Return(z) | {} |
| 块末尾 | {} |

*表 19-2：到达清单 19-19 中各指令的复制*

我们正确地认识到 Store 指令可能覆盖 x，因此不能在 z = x + y 中把 x 替换为 1。我们还假设 Store 指令可能覆盖 y，因为我们的分析没有聪明到能意识到 ptr1 不可能指向 y。于是，尽管这样做是安全的，我们也不会在 z = x + y 中把 y 替换为 2。我们再次做出了保守假设：会错过一些安全的优化，但绝不会应用任何不安全的优化。

#### 实现取地址分析

我们刚才用来识别别名变量的方法叫取地址分析（address-taken analysis）。执行这种分析时，我们检查 TACKY 函数中的每条指令，找出每个要么具有静态存储期、要么被 GetAddress 指令取了地址的变量。（我们假设所有静态变量都是别名变量，因为其他函数可能取了它们的地址。）优化流水线每迭代一轮，我们都要重新执行这种分析，因为一旦优化掉了某些 GetAddress 指令，结果就可能改变。清单 19-20 展示了它如何融入清单 19-6 定义的整个优化流水线。

```text
optimize(function_body, enabled_optimizations):
    --snip--
    while True:
        aliased_vars = address_taken_analysis(function_body)
        --snip--
        if enabled_optimizations contains "COPY_PROP":
            cfg = copy_propagation(cfg, aliased_vars)
        if enabled_optimizations contains "DEAD_STORE_ELIM":
            cfg = dead_store_elimination(cfg, aliased_vars)
        --snip--
```

*清单 19-20：把取地址分析加入 TACKY 优化流水线*

取地址分析只是别名分析（alias analysis，又称指针分析，pointer analysis）的一种，后者试图判定两个指针或变量能否指涉同一个对象。大多数指针分析算法都比取地址分析强大。例如，它们能推断出清单 19-19 中的 ptr1 永远不会指向 y。

#### 更新传递函数

接下来扩展传递函数，以支持第二部分添加的新类型和新指令。清单 19-21 给出了我们改进后的新传递函数。它复现了清单 19-15，其中支持额外类型的改动以粗体标出。

```text
transfer(block, initial_reaching_copies, aliased_vars):
    current_reaching_copies = initial_reaching_copies

    for instruction in block.instructions:
        annotate_instruction(instruction, current_reaching_copies)
        match instruction with
        | Copy(src, dst) ->
            --snip--
            if (get_type(src) == get_type(dst)) or (signedness(src) == signedness(dst)):
                current_reaching_copies.add(instruction)
        | FunCall(fun_name, args, dst) ->
            for copy in current_reaching_copies:
                if (copy.src is in aliased_vars
                    or copy.dst is in aliased_vars
                    or (dst is not null and (copy.src == dst or copy.dst == dst))):
                    current_reaching_copies.remove(copy)
        | Store(src, dst_ptr) ->
            for copy in current_reaching_copies:
                if (copy.src is in aliased_vars) or (copy.dst is in aliased_vars):
                    current_reaching_copies.remove(copy)
        | Unary(operator, src, dst) or any other instruction with dst field ->
            --snip--
        | _ -> continue

    annotate_block(block.id, current_reaching_copies)
```

*清单 19-21：支持第二部分特性的到达复制分析传递函数*

这个清单中的大部分改动我们已经谈过。在把 Copy 指令加入 current_reaching_copies 之前，我们要确保其源和目的地类型相同，或至少符号性（signedness）相同。signedness 辅助函数应把 char 算作有符号类型、把所有指针类型算作无符号类型，这样我们就能在 char 与 signed char 之间、不同指针类型之间、以及指针与 unsigned long 之间传播复制。（符号性的概念不适用于 double 和非标量类型。没关系，因为我们不用 Copy 指令转换为或转换自这些类型。如果 Copy 用到 double 或非标量操作数，两个操作数的类型一定相同，无需检查符号性。）

遇到函数调用或 Store 指令时，杀死所有与别名变量相关的复制。我们还要考虑函数调用可能没有目的操作数这一事实。注意，我们不杀死 Store 指令的 dst_ptr 操作数。Store 不会改变目的指针本身的值，只改变它所指对象的值。最后，遇到第二部分添加的其他任何指令——包括类型转换、CopyToOffset 和 CopyFromOffset——都杀死与其目的地相关的复制。我们不追踪结构或数组中单个子对象的复制，所以 CopyToOffset 和 CopyFromOffset 只会杀死到达复制，不会产生新的。

#### 更新 rewrite_instruction

第二部分的大部分新 TACKY 指令都按第一部分指令的同样方式改写：替换任何由到达复制定义的源操作数。唯一的例外是 GetAddress，我们永远不改写它。对 GetAddress 应用复制传播没有意义，因为它使用的是源操作数的地址而非其值。

**测试复制传播遍**

如果你只完成了第一部分，运行下面的命令测试复制传播实现：

```bash
$ ./test_compiler /path/to/your_compiler --chapter 19 --propagate-copies
--int-only
```

如果你完成了第一和第二部分，运行：

```bash
$ ./test_compiler /path/to/your_compiler --chapter 19 --propagate-copies
```

## 死存储消除

我们的最后一个优化是死存储消除。我们将使用活性分析——一种反向数据流分析——计算被优化函数中每个位置上哪些变量是活的，然后利用分析结果找出并消除死存储。

变量在某个位置是活的（live），指的是它在那个位置的值可能在程序之后被读取；否则它就是死的（dead）。更精确地说，变量 x 在给定位置 p 活，需满足两个条件。第一，从 p 到之后某条使用 x 的指令之间至少存在一条路径。我们说 x 被任何使用它的指令生成（generate）。第二，x 在从 p 到那条指令的路径上不被更新。我们说 x 被任何更新它的指令杀死（kill），正如到达复制在其任一操作数被更新时被杀死一样。（在大多数数据流分析的讨论中——不只是本章这两种——你都会见到生成和杀死这对术语。）看看图 19-10 中的控制流图。

*图 19-10：x 在定义之后立即处于活状态的控制流图*

从 x = 10 到 EXIT 有两条路径。经过 B1 的路径上 x 从未被使用；经过 B2 的路径上，x 在一条 Return 指令中被使用。我们知道 x 在 x = 10 之后的那一点是活的，因为它在某条路径上被生成。同理，x 在 B0 末尾、B2 开头以及 B2 中 Return 指令之前也都是活的。另一方面，x 在 B2 末尾以及 B1 中的每个位置都是死的，因为从那些位置出发没有任何路径通向使用 x 的指令。注意 x 在 B0 的最开头也是死的，因为在 x = 10 给它赋新值之前，我们没有使用它的（未初始化的）值。

再看图 19-11 中的控制流图。

*图 19-11：存在对 x 的死存储的控制流图*

这里同样有两条从 x = 10 到 EXIT 的路径。两条路径都经过使用 x 的 Return 指令，但在两条路径上，x 都在 x = 10 与生成它的 Return 指令之间被杀死。这意味着 x 在 x = 10 之后立刻就是死的。在这张控制流图中，它只在两个位置是活的：B1 中 x = f() 之后和 B2 中 x = g() 之后。

如果一条指令给一个死变量赋值且没有其他副作用，它就是死存储。因此，x = 10 在图 19-11 中是死存储，在图 19-10 中不是。注意，我们在意的是变量在指令执行之后（而不是之前）是否是死的。在代码片段

```text
x = x + 1
Return(0)
```

中，x 在 x = x + 1 之前是活的，但在它之后是死的。x 在更新之后立刻就是死的，说明这条指令是死存储，可以消除。

### 活性分析

与每种数据流分析一样，活性分析需要传递函数、汇聚算子和迭代算法。由于这是一个反向流动的问题，传递函数将从基本块的末尾开始向前推到开头，而不像到达复制分析那样从头到尾处理。类似地，汇聚算子从块的后继（而非前驱）收集信息。我们还会使用一个略有不同的迭代算法在控制流图中向后传递数据。下面逐一细看这些部件。

#### 传递函数

传递函数接受基本块末尾的活变量集合，计算每条指令之前哪些变量是活的。如图 19-10 和图 19-11 所示，指令生成它读取的所有变量，杀死它更新的所有变量。例如，要计算指令 x = y * z 之前的活变量，我们取该指令之后一刻的活变量集合，加上 y 和 z，减去 x。如果一条指令既读又写同一个变量，它生成该变量而不是杀死它。例如，x = x + 1 生成 x。

把传递函数应用到清单 19-22 的基本块上。

```text
x = 4
x = x + 1
y = 3 * x
Return(y)
```

*清单 19-22：一个基本块*

传递函数从基本块底部向上处理。假设块末尾（Return 指令之后）没有活变量。（如果函数涉及静态变量，这个假设可能不成立，稍后再处理。）处理 Return 指令时，把 y 加入活变量集合。接着 y = 3 * x 杀死 y、生成 x。下一条指令 x = x + 1 生成 x，这没有影响，因为 x 已经是活的。最后 x = 4 杀死 x，基本块开头就没有活变量了。表 19-3 总结了清单 19-22 中每条指令之后一刻的活变量。

| 指令 | 活变量 |
| --- | --- |
| 块开头 | {} |
| x = 4 | {x} |
| x = x + 1 | {x} |
| y = 3 * x | {y} |
| Return(y) | {} |

*表 19-3：清单 19-22 中每条指令之后的活变量*

静态变量把事情复杂化了，与到达复制分析中的情况如出一辙。我们不知道其他函数会如何与遇到的静态变量交互：可能读它们、更新它们，或两者皆有。我们假设每个函数都会读取每个静态变量。这个假设是保守的，因为它阻止我们消除对这些变量的较早写入。清单 19-23 给出传递函数的伪代码。

```text
transfer(block, end_live_variables, all_static_variables):
  ❶ current_live_variables = end_live_variables

  ❷ for instruction in reverse(block.instructions):
      ❸ annotate_instruction(instruction, current_live_variables)

        match instruction with
        | Binary(operator, src1, src2, dst) ->
            current_live_variables.remove(dst)
            if src1 is a variable:
                current_live_variables.add(src1)
            if src2 is a variable:
                current_live_variables.add(src2)
        | JumpIfZero(condition, target) ->
            if condition is a variable:
                current_live_variables.add(condition)
        | --snip--
        | FunCall(fun_name, args, dst) ->
            current_live_variables.remove(dst)
            for arg in args:
                if arg is a variable:
                    current_live_variables.add(arg)
          ❹ current_live_variables.add_all(all_static_variables)

  ❺ annotate_block(block.id, current_live_variables)
```

*清单 19-23：活性分析的传递函数*

我们从块末尾的活变量集合出发 ❶，然后按逆序处理指令列表 ❷。我们给每条指令标注它执行之后一刻的活变量集合 ❸；稍后判断它是否为死存储时会用到这个标注。然后计算该指令之前一刻哪些变量是活的：先杀死它的目的地（如果有的话），再加入它读取的每个变量。清单 19-23 包含处理 Binary 指令（更新一个操作数、读取另外两个）和 JumpIfZero 指令（读取一个操作数但不更新任何东西）的伪代码，其余大多数指令的伪代码被省略，因为它们遵循同样的模式。FunCall 是唯一的特例：照常杀死其目的地、加入其实参，但同时要加入所有静态变量 ❹。最后，用第一条指令之前的活变量标注整个块 ❺。汇聚算子稍后会用到这个信息。

计算 all_static_variables 有几种办法。一种是在开始死存储消除遍之前扫描这个 TACKY 函数、找出静态变量。另一种是扫描整个符号表找静态变量，不必关心哪些变量出现在哪些函数中。这里多加一些多余的静态变量没有害处，因为它们不会改变我们最终消除哪些指令。

#### 汇聚算子

汇聚算子计算基本块末尾哪些变量是活的。要找某个块 B 末尾的活变量，我们要看它的所有后继。如果一个变量在至少一个后继的开头是活的，那么它在 B 的末尾也必定是活的，因为从 B 的末尾至少有一条路径经过那个后继到达生成该变量的指令。基本上，我们对所有后继开头的活变量取集合并集。

我们假设每个静态变量在 EXIT 节点都是活的：其他函数（或当前函数的其他调用）可能读取这些变量。自动存储期的变量在 EXIT 都是死的，因为离开函数后就无法访问它们了。清单 19-24 的伪代码定义了汇聚算子。

```text
meet(block, all_static_variables):
    live_vars = {}
    for succ_id in block.successors:
        match succ_id with
        | EXIT -> live_vars.add_all(all_static_variables)
        | ENTRY -> fail("Malformed control-flow graph")
        | BlockId(id) ->
            succ_live_vars = get_block_annotation(succ_id)
            live_vars.add_all(succ_live_vars)

    return live_vars
```

*清单 19-24：活性分析的汇聚算子*

在到达复制分析中，我们要找的是在通往某点的每条路径上都出现的复制，所以用集合交集作汇聚算子。在活性分析中，我们想知道变量是否在从某点出发的任意一条路径上被使用，所以改用集合并集。这与一个分析是前向、另一个是反向无关。有些前向分析使用集合并集，因为它们关心的是通往某点的至少一条路径是否具有某性质；有些反向分析使用集合交集，因为它们关心的是从某点出发的每条路径是否都具有某性质。还有一些更复杂的分析既不用并集也不用交集，而是使用完全不同的汇聚算子。

#### 迭代算法

最后实现活性分析的迭代算法。它与清单 19-17 中的迭代算法有几处不同。第一，当某个块的标注发生变化时，我们把它的前驱（而不是后继）加入工作列表。第二，我们使用不同的块初始标注。回想一下，每个块的初始标注应当是汇聚算子的单位元；我们的汇聚算子是集合并集，所以初始标注是空集。随着分析越来越多从块到程序中靠后位置的路径，我们会往这个集合里加入更多的活变量。

反向迭代算法的伪代码我就不给了，因为它与我们已定义的前向算法太相似了。不过我可以给你几条实现建议。第一，可以考虑按后序初始化工作列表。（回想一下，后序排序是通过深度优先遍历、在每个节点的后继都访问过之后再访问该节点得到的。）这能让反向算法更快终止，正如按逆后序初始化工作列表能让前向算法更快终止一样。这种排序意味着：只要可能，每个块都要等到它的所有后继都访问过之后才被访问。

第二条建议是让反向迭代算法可复用。下一章我们会再次实现活性分析，不过这次是针对汇编程序。汇聚算子和传递函数的细节会变，迭代算法不会变。试着组织好代码，使同一个迭代算法能搭配不同的汇聚算子和传递函数复用；这样下一章你就能直接用它分析汇编程序。

### 删除死存储

运行完活性分析后，我们找出 TACKY 函数中的所有死存储并删除它们。如果一条指令的目的地在执行之后立刻就是死的，它就是死存储，比如下面这个例子：

```text
x = 1
x = 2
```

活性分析会告诉我们，x 在 x = 1 之后立刻就是死的，因此删除那条指令是安全的。我们绝不删除函数调用，即使它们更新的是死变量，因为它们可能有其他副作用。我们也不删除没有目的地的指令，比如 Jump 和 Label。清单 19-25 演示了如何识别死存储。

```text
is_dead_store(instr):
    if instr is FunCall:
        return False

    if instr has a dst field:
        live_variables = get_instruction_annotation(instr)
        if instr.dst is not in live_variables:
            return True

    return False
```

*清单 19-25：识别死存储*

如果你只完成了第一部分，关于死存储消除你需要了解的就都齐了！可以直接跳到测试套件。否则，请继续阅读，了解如何处理第二部分添加的类型和指令。

### 支持第二部分的 TACKY 程序

更新传递函数时，需要想清楚每条新指令可能生成或杀死哪些活变量。类型转换指令（如 Truncate 和 SignExtend）很简单：每条都生成其源操作数、杀死其目的地，与我们已处理的 Copy 和 Unary 指令非常相似。AddPtr 也遵循常规模式：生成两个源操作数、杀死其目的地。

指针和聚合类型上的操作更棘手。指针带来的问题与到达复制分析中的本质上相同：通过指针读写时，我们分不清访问的是哪个底层对象。拿不准时，我们应偏向保守，宁可认为变量是活的。因此，通过指针读取应生成所有别名变量，而通过指针写入不应杀死其中任何一个。对聚合变量（aggregate variable）我们采取类似做法：读取聚合变量的一部分会生成它，但更新它的一部分不会杀死它。更新版传递函数的伪代码我就不给了；关键点都讲完之后，剩下的细节留给你自己完成。汇聚算子不变；特别地，静态变量在 EXIT 仍然是活的，但其他别名变量不是，因为它们的生命周期在函数返回时就结束了。

最后，更新这个优化的最后一步——用活性分析的结果找出并消除死存储。我们绝不消除 Store 指令，因为我们不知道它的目的地是否是死的。即便当前函数中的每个变量都是死的，Store 仍然可能有可见的副作用。例如，它可能更新定义在另一个函数中的对象，就像下面这个例子：

```text
update_through_pointer(param):
    Store(10, param)
    Return(0)
```

Store 指令之后，update_through_pointer 中没有任何活变量。但那条指令显然不是死存储；它更新了一个我们的分析没有追踪、但很可能在程序之后被读取的对象。

第二部分的其他所有指令——包括 Load、GetAddress、CopyToOffset 和 CopyFromOffset——都适用识别死存储的常规逻辑。

**测试整个优化流水线**

既然实现了整个优化流水线，你可以测试它了。你可能想先只测试死存储消除遍。如果你只完成了第一部分，可以用下面的命令：

```bash
$ ./test_compiler /path/to/your_compiler --chapter 19 --eliminate-dead-stores
--int-only
```

如果你完成了第一和第二部分，运行：

```bash
$ ./test_compiler /path/to/your_compiler --chapter 19 --eliminate-dead-stores
```

--eliminate-dead-stores 选项会运行 tests/chapter_19/dead_store_elimination 中死存储消除专用的测试，还会在启用全部四种优化的情况下运行前几章的测试。

要测试整条优化流水线，运行

```bash
$ ./test_compiler /path/to/your_compiler --chapter 19 --int-only
```

或：

```bash
$ ./test_compiler /path/to/your_compiler --chapter 19
```

这条命令会运行所有单个优化的测试，并在启用全部四种优化的情况下运行前几章的所有测试（一如既往，--int-only 选项会排除依赖第二部分语言特性的测试）。它还会运行 tests/chapter_19/whole_pipeline 中的测试，这些测试关注不同优化如何协同工作，验证每个优化都利用了其他优化创造的机会。

## 小结

本章中，你实现了四种重要的编译器优化：常量折叠、不可达代码消除、复制传播和死存储消除。你学会了这些优化如何协同工作来变换程序的 TACKY 表示，使你的编译器生成比以往更小、更快、更简洁的汇编代码。你还学会了如何构造控制流图、如何执行数据流分析。这些技术是许多不同优化的基础，远不限于本章介绍的这几种。如果你将来想自己实现更多 TACKY 优化，你已经做好了充分准备。

下一章，你将编写寄存器分配器。你将使用图着色算法把伪寄存器映射到硬件寄存器，学会在图着色失败、寄存器用尽时如何溢出（spill）寄存器。你还将使用一种叫寄存器合并（register coalescing）的技术清理汇编代码中许多不必要的 mov 指令。到那一章结束时，你的汇编程序仍然和生产级编译器生成的不太一样，但会接近得多。

## 延伸资源

本节列出我编写本章时参考过的资源，按主题组织。

**编译器优化的安全影响**

Zhaomo Yang 等人的《Dead Store Elimination (Still) Considered Harmful》调查了程序员为避免不想要的死存储消除而尝试的各种方法以及每种方法的局限（https://www.usenix.org/system/files/conference/usenixsecurity17/sec17-yang.pdf）。

Vijay D'Silva、Mathias Payer 和 Dawn Song 的《The Correctness-Security Gap in Compiler Optimization》考察了几种不同编译器优化的安全影响，并形式化了优化应当保持的某些安全性质（https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=7163211）。

**数据流分析**

Alfred V. Aho 等人的《Compilers: Principles, Techniques, and Tools》第 2 版（Addison-Wesley，2006）第 9 章比我这里更严格地定义了数据流分析。它还证明了迭代算法正确且会在合理时间内终止，并讨论了逆后序遍历（书中称之为深度优先排序）在该算法中的使用。

Paul Hilfinger 在 UC Berkeley CS164 课程的讲义幻灯片用大量例子概述了同样的内容（https://inst.eecs.berkeley.edu/~cs164/sp11/lectures/lecture37-2x2.pdf）。我觉得这些幻灯片中对活性分析的解释特别有帮助。

Eli Bendersky 的博文《Directed Graph Traversal, Orderings and Applications to Data-Flow Analysis》介绍了如何将图按后序和逆后序排序以加速数据流分析（https://eli.thegreenplace.net/2015/directed-graph-traversal-orderings-and-applications-to-data-flow-analysis）。

**复制传播**

关于到达复制分析的每一次讨论，表述似乎都略有不同。本章的版本参考了 Jeffrey Ullman 关于《Compilers: Principles, Techniques, and Tools》的课堂讲义（http://infolab.stanford.edu/~ullman/dragon/slides3.pdf 和 http://infolab.stanford.edu/~ullman/dragon/slides4.pdf）。

删除多余复制的想法借鉴自 LLVM 的底层复制传播遍（https://llvm.org/doxygen/MachineCopyPropagation_8cpp_source.html）。

**别名分析**

Phillip Gibbons 在 Carnegie Mellon 编译器优化课程中的讲义幻灯片对别名分析算法做了快速概述（https://www.cs.cmu.edu/afs/cs/academic/class/15745-s16/www/lectures/L16-Pointer-Analysis.pdf）。

