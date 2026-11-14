# 编码风格与 gdb 调试

好的编码风格让代码更易读、易维护。调试是找到并修复bug的过程，gdb是C语言最常用的调试工具。

## 代码风格规范

### 缩进

统一使用缩进表示代码层次，推荐4个空格或1个Tab。

```c
// 推荐
if (x > 0) {
    printf("positive\n");
}

// 不推荐：缩进混乱
if (x > 0) {
printf("positive\n");
    }
```

花括号的两种风格：

```c
// K&R风格：左花括号不换行
if (x > 0) {
    printf("positive\n");
}

// Allman风格：左花括号换行
if (x > 0)
{
    printf("positive\n");
}
```

团队项目应保持一致风格。

### 命名

变量和函数名应清晰表达用途。

```c
// 好的命名
int student_count;
float average_score;
void calculate_total();

// 不好的命名
int x;
float f;
void func();
```

常见命名风格：

| 风格 | 示例 | 适用场景 |
|------|------|----------|
| 下划线 | `student_count` | 变量、函数 |
| 小驼峰 | `studentCount` | 变量、函数 |
| 大驼峰 | `StudentCount` | 结构体、类型 |
| 全大写 | `MAX_SIZE` | 宏、常量 |

一些约定：

- 宏和常量用全大写
- 变量名用小写开头
- 结构体名用大写开头
- 布尔变量可用`is_`前缀：`is_empty`

### 注释

注释解释"为什么"，而不是"是什么"。

```c
// 不好的注释：解释显而易见的事
i++;  // i加1

// 好的注释：解释原因
i++;  // 跳过头部字段

// 好的注释：解释复杂逻辑
// 使用二分查找优化，时间复杂度O(log n)
int index = binary_search(arr, len, target);
```

函数注释说明功能、参数和返回值：

```c
/**
 * 计算数组的平均值
 * @param arr 数组指针
 * @param len 数组长度
 * @return 平均值，len<=0时返回0
 */
float average(int arr[], int len) {
    if (len <= 0) return 0;
    float sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum / len;
}
```

### 其他建议

**控制行宽**：每行不超过80-100字符，过长的行应该换行。

```c
// 换行示例
int result = some_function(arg1, arg2,
                           arg3, arg4);
```

**避免魔法数字**：用有意义的常量替代。

```c
// 不好
if (score >= 60) { ... }

// 好
#define PASS_SCORE 60
if (score >= PASS_SCORE) { ... }
```

**函数不要太长**：一个函数控制在50行以内，超过则考虑拆分。

## gdb 基本使用

gdb是GNU调试器，用于调试C程序。

### 启动调试

编译时加`-g`选项生成调试信息：

```bash
gcc -g program.c -o program
```

启动gdb：

```bash
gdb ./program
```

进入gdb后显示`(gdb)`提示符，等待输入命令。

### 常用命令

| 命令 | 简写 | 作用 |
|------|------|------|
| `run` | `r` | 运行程序 |
| `break` | `b` | 设置断点 |
| `next` | `n` | 单步执行（不进入函数） |
| `step` | `s` | 单步执行（进入函数） |
| `continue` | `c` | 继续运行到下一个断点 |
| `print` | `p` | 打印变量值 |
| `list` | `l` | 显示源代码 |
| `quit` | `q` | 退出gdb |

## 断点设置与单步执行

### 设置断点

```bash
# 在函数处设置断点
(gdb) break main
(gdb) b main

# 在行号处设置断点
(gdb) break 20
(gdb) b 20

# 在文件和行号处设置断点
(gdb) break program.c:15

# 条件断点
(gdb) break 20 if x > 10

# 查看所有断点
(gdb) info break

# 删除断点
(gdb) delete 1  # 删除1号断点
(gdb) delete    # 删除所有断点
```

### 单步执行

```bash
# 运行到第一个断点
(gdb) run

# 单步执行（逐过程，不进入函数）
(gdb) next
(gdb) n

# 单步执行（逐语句，进入函数）
(gdb) step
(gdb) s

# 继续运行到下一个断点
(gdb) continue
(gdb) c

# 运行到当前函数返回
(gdb) finish
```

## 查看变量与内存

### 查看变量

```bash
# 打印变量值
(gdb) print x
(gdb) p x

# 打印数组
(gdb) p arr[0]@5    # 打印arr[0]到arr[4]
(gdb) p *arr@5      # 另一种写法

# 打印结构体
(gdb) p stu

# 打印变量类型
(gdb) ptype x

# 监视变量（变量变化时自动显示）
(gdb) watch x
(gdb) info watch
```

### 查看内存

```bash
# 查看内存内容
# x/格式 地址
(gdb) x/10x 0x7fff5fbff8c   # 以十六进制显示10个单位
(gdb) x/10i main             # 显示main函数的10条指令

# 格式字符：
# x - 十六进制
# d - 十进制
# c - 字符
# s - 字符串
# i - 指令
```

### 查看调用栈

```bash
# 查看调用栈
(gdb) backtrace
(gdb) bt

# 切换栈帧
(gdb) frame 1
(gdb) up      # 上移一层
(gdb) down    # 下移一层
```

## 常见调试技巧

### 定位段错误

程序崩溃时，gdb会停在崩溃位置：

```bash
$ gdb ./program
(gdb) run

Program received signal SIGSEGV, Segmentation fault.
0x0000000000401156 in main () at program.c:15
15          *ptr = 10;

# 查看崩溃位置
(gdb) bt
(gdb) frame 0
(gdb) p ptr    # 发现ptr是NULL或无效地址
```

### 调试循环

在循环中设置条件断点：

```bash
# 循环第100次时停止
(gdb) b 15 if i == 100

# 或者手动在循环中停止
(gdb) b 15
(gdb) c
# ... 多次continue直到想要的位置
```

### 检查内存泄漏

配合valgrind工具：

```bash
gcc -g program.c -o program
valgrind --leak-check=full ./program
```

### 调试核心转储

程序崩溃时生成core文件：

```bash
# 启用核心转储
ulimit -c unlimited

# 运行程序（崩溃后生成core文件）
./program

# 用gdb分析core文件
gdb ./program core
(gdb) bt
```

### 实用技巧

**设置显示选项**：

```bash
(gdb) set print pretty on    # 美化结构体输出
(gdb) set print array on     # 美化数组输出
```

**执行shell命令**：

```bash
(gdb) shell ls
(gdb) !ls                    # 另一种写法
```

**在gdb中调用函数**：

```bash
(gdb) call some_function(arg1, arg2)
```

**附加到运行中的进程**：

```bash
# 查找进程ID
ps aux | grep program

# 附加调试
gdb -p <pid>
# 或
gdb -p <pid>
```
