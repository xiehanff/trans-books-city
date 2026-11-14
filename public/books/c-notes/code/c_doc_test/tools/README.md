# C 文档代码块清单工具

`verify_manifest.py` 不依赖 Markdown 第三方库，只解析 `C笔记` 目录下的顶层 `*.md` 文件中的 fenced code block。它会检查清单是否与当前文档完全一致：包括文件、围栏序号、围栏位置、语言、起止行和 SHA-256。代码块内容按 UTF-8 逻辑行以 LF 连接，去掉围栏前的那个终止换行后计算哈希，因此 CRLF/LF 检出不会造成无意义漂移。

## 命令

在仓库根目录执行：

```text
python3 MarkDown笔记/C笔记/c_doc_test/tools/verify_manifest.py --check
python3 MarkDown笔记/C笔记/c_doc_test/tools/verify_manifest.py --summary
python3 MarkDown笔记/C笔记/c_doc_test/tools/verify_manifest.py --compile-fail
```

- `--check` 成功返回 0，清单遗漏、重复、位置变化、语言变化、mode 变化或哈希漂移返回 1，文件/JSON/Markdown 解析错误返回 2，可直接作为 CTest 的检查命令。
- `--summary` 输出当前代码块、语言和 C mode 统计，不修改文件。
- `--compile-fail` 使用严格 C11 编译器逐个检查 `compile_fail` 代码块必须失败；不会运行 UB 示例。
- `--update` 按当前文档重新生成 `manifest.json`。它只适合文档有意修改后使用，生成后应人工复核新 C mode；日常校验不要使用该选项。
- `--source-dir` 和 `--manifest` 可用于对另一份笔记目录或清单进行校验。

清单中的 ID 是 `文件名 + 围栏序号 + 语言` 的 SHA-256 前缀，内容改变不会悄悄复用另一段代码的 ID；同一文件中的每个围栏都有唯一位置键。清单登记所有语言的 fenced block，未标语言的块归为 `text`，并保留原始 `info_string`。

## C mode 分类原则

mode 是**保守的意图标签**，不是“已经编译/运行成功”的声明。当前分类由 `heuristic-v1` 生成，所有 C block 都有非空 mode，危险示例不会因为含有 `main` 就自动成为普通运行测试。

| mode | 用途 |
| --- | --- |
| `run` | 有 `main` 且没有识别出的危险行为或平台依赖，意图上可作为运行示例；仍需补齐头文件、输入和依赖后才能验证。 |
| `compile_pass` | 函数、类型或实现片段，看起来适合编译检查，但通常没有 `main`；不保证脱离上下文即可编译。 |
| `compile_fail` | 明确展示非法声明、错误语法或预期编译失败的片段。 |
| `platform` | 依赖 x86/ABI、操作系统头文件、GCC/Clang 扩展、SIMD、线程或汇编等环境的片段。 |
| `ub_demo` | 展示越界、空/野/悬空指针、泄漏、重复释放、栈溢出、未定义行为等风险的片段；禁止当作普通运行测试。 |
| `fragment` | 语句、表达式或控制流等需要拼接到更大程序中的教学片段。 |
| `document_only` | 伪代码、声明/签名、输出或主要用于解释语法的内容，不应自动编译或运行。 |

分类优先级为 `ub_demo`、`platform`、`compile_fail`，再判断 `run`，最后在 `compile_pass`、`fragment`、`document_only` 中选择。这个优先级的目的，是宁可少自动化，也不要把危险代码送入运行测试。

## 当前清单统计

生成清单时扫描了 39 个 Markdown 文件，共 917 个 fenced block：

- C：704
- bash：63
- asm：34
- makefile：31
- gdb：6
- text：79（其中 72 个未标语言，7 个显式标为 `text`）

704 个 C block 的 mode 统计为：`fragment` 325、`compile_pass` 200、`ub_demo` 60、`document_only` 42、`run` 38、`platform` 32、`compile_fail` 7。统计只说明清单分类结果，不表示这些片段已经成功编译；真正的编译/运行验证仍应另行添加针对性的测试。
