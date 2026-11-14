# C 笔记代码验证项目

这个目录用于验证 `MarkDown笔记/C笔记` 中的教程代码。验证分为三层：

1. `lib/` 中整理出的安全核心实现，由 `test/` 中的 C11 单元测试覆盖；
2. `manifest.json` 追踪全部 Markdown fenced code block，并校验位置、语言和 SHA-256；
3. `platform/` 与 `tools/check_platform_examples.py` 验证 Linux/x86-64、预处理、链接、GNU Make、汇编语法和安全 Bash 示例。

## 覆盖范围

当前清单覆盖 C 笔记目录的 **39 个 Markdown 文件、917 个代码块，其中 704 个 C 代码块**。每个 C 代码块都有明确分类：`run`、`compile_pass`、`compile_fail`、`platform`、`ub_demo`、`fragment` 或 `document_only`。

安全、可运行的教程核心内容已整理为以下测试专题：

| 测试参数 | 对应章节 | 验证内容 |
|---|---|---|
| `basic_examples` | 01–05、07、09–10 | 基础语法、类型、表达式、函数、递归、结构体、数组和字符串 |
| `control_flow` | 06、08 | 分支、`switch`、`while`、`do while`、`for`、`break`、`continue` |
| `number_examples` | 02–04、15–16 | 八/十六进制、无符号表示、回绕和数值边界 |
| `architecture_examples` | 17、S02 | 字节序、字节复制、指针和结构体布局关系 |
| `sort_search` | 12 | 冒泡、选择、插入、快速、归并排序及查找变体 |
| `stack_queue` | 13 | 栈、括号匹配、普通队列、循环队列和双栈队列 |
| `bit_operations` | 16 | 位获取、设置、清除、翻转、标志位和 RGB 分量 |
| `struct_array` / `string_examples` | 09–10 | 结构体数组和安全字符串操作 |
| `interface_examples` | 23–24、S01 | 指针、`const`、多级指针、输出参数、比较器和可变参数 |
| `stdlib_examples` | 25、内存函数、S04–S05 | `memcpy`/`memmove`/`memset`、字符串、动态内存和安全宏 |
| `linked_list` / `function_pointer` / `dynamic_array` | S06–S08 | 链表、函数指针/回调、堆内存和动态数组 |
| `http_protocol` | S09 | HTTP/1.1 请求行解析、响应构造和长度边界 |
| `json_parser` / `string_builder` / `memory_pool` / `generic_vector` | M01–M04 | JSON、动态字符串、内存池/Arena 和泛型容器 |

## 运行全部验证

在当前目录执行：

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

CTest 包含 19 组 C 单元测试、Markdown 清单校验、故意编译失败示例校验和平台示例校验，共 22 个用例。也可以直接运行全部 C 单元测试：

```bash
./build/c_doc_tests
```

只运行一个专题：

```bash
./build/c_doc_tests json_parser
./build/c_doc_tests sort_search
./build/c_doc_tests linked_list
```

单独校验 Markdown 清单：

```bash
python3 tools/verify_manifest.py --check
python3 tools/verify_manifest.py --summary
```

单独校验故意编译失败示例：

```bash
python3 tools/verify_manifest.py --compile-fail
```

单独校验平台示例：

```bash
python3 tools/check_platform_examples.py --check
```

## Sanitizer 验证

本项目默认使用严格警告编译。需要进行 AddressSanitizer/UBSan 复验时，可使用支持 sanitizer 运行库的 Clang：

```bash
CC=clang cmake -S . -B /tmp/c_doc_test-sanitize \
  -DCMAKE_C_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer -g' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build /tmp/c_doc_test-sanitize --parallel
ASAN_OPTIONS=detect_leaks=1 \
  ctest --test-dir /tmp/c_doc_test-sanitize --output-on-failure
```

## 危险和平台相关示例

文章中用于展示错误后果的代码（越界、Use-after-free、修改字符串字面量、死循环、整数溢出等）不会直接运行；它们在 `manifest.json` 中标为 `ub_demo` 或 `compile_fail`，避免把未定义行为当成测试结果。

x86 汇编、GCC 扩展、Linux/POSIX、Makefile 和 Bash 示例不会伪装成可移植 C11 单元测试，而是由平台夹具按能力执行；当前环境不具备对应工具或架构时会明确输出 `[SKIP]`。

修改 Markdown 中的代码后，先运行 `verify_manifest.py --check`；清单哈希发生变化时，应重新审阅分类，再运行完整 CTest。`build/` 是 CMake 生成目录，已加入 `.gitignore`，不应提交。
