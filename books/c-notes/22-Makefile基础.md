# 第22章 Makefile 基础

make 是一个自动化构建工具，Makefile 定义了构建规则。掌握 Makefile 对于管理 C 项目至关重要。

## make 工具简介

make 根据文件的时间戳判断是否需要重新构建，避免不必要的编译。

```bash
make          # 使用默认的 Makefile 或 makefile
make -f build.mk  # 指定 makefile 文件
make target   # 构建指定目标
make clean    # 执行 clean 目标
```

## Makefile 基本结构

### 基本语法

```makefile
目标: 依赖列表
	命令
```

注意：命令前必须是 Tab，不能是空格。

```makefile
hello: hello.c
	gcc hello.c -o hello
```

运行 `make hello` 时：
1. 检查 `hello` 是否存在
2. 如果不存在，或 `hello.c` 比 `hello` 新，则执行命令

### 多规则示例

```makefile
program: main.o utils.o
	gcc main.o utils.o -o program

main.o: main.c utils.h
	gcc -c main.c -o main.o

utils.o: utils.c utils.h
	gcc -c utils.c -o utils.o

clean:
	rm -f *.o program
```

构建过程：
```
make program
  -> 检查 main.o（依赖 main.c 和 utils.h）
     -> 如果需要更新，执行 gcc -c main.c
  -> 检查 utils.o（依赖 utils.c 和 utils.h）
     -> 如果需要更新，执行 gcc -c utils.c
  -> 链接生成 program
```

### 伪目标

`.PHONY` 声明不对应文件的目标：

```makefile
.PHONY: clean all install

clean:
	rm -f *.o program

all: program doc

install: program
	cp program /usr/local/bin/
```

## 目标、依赖、规则

### 默认目标

make 默认构建第一个目标：

```makefile
all: program doc

program:
	@echo "Building program"

doc:
	@echo "Building doc"
```

### 依赖关系

依赖可以是其他目标或文件：

```makefile
program: main.o utils.o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) -c $< -o $@

utils.o: utils.c
	$(CC) -c $< -o $@
```

### 空命令

有时只需要标记依赖关系：

```makefile
utils.o: utils.h
main.o: utils.h
```

这表示 `utils.o` 和 `main.o` 都依赖 `utils.h`，但没有额外命令。可以与规则分开写：

```makefile
main.o: main.c
	gcc -c main.c

main.o: utils.h config.h  # 额外的依赖
```

## 自动变量

make 提供自动变量简化规则：

| 变量 | 含义 |
|------|------|
| `$@` | 目标文件名 |
| `$<` | 第一个依赖文件名 |
| `$^` | 所有依赖文件名（去重） |
| `$+` | 所有依赖文件名（保留重复） |
| `$*` | 不含扩展名的目标文件名 |
| `$?` | 比目标新的依赖文件列表 |

### 使用示例

```makefile
program: main.o utils.o
	gcc $^ -o $@

main.o: main.c
	gcc -c $< -o $@

%.o: %.c
	gcc -c $< -o $@
```

## 模式规则

使用 `%` 作为通配符定义通用规则：

```makefile
# 所有 .o 文件由对应的 .c 文件生成
%.o: %.c
	$(CC) -c $< -o $@

# 所有 .d 文件由对应的 .c 文件生成（依赖关系）
%.d: %.c
	$(CC) -MM $< > $@
```

### 静态模式规则

限定模式适用的目标：

```makefile
objects = main.o utils.o debug.o

$(objects): %.o: %.c
	$(CC) -c $< -o $@
```

## 变量

### 定义变量

```makefile
CC = gcc
CFLAGS = -Wall -g -O2
LDFLAGS = -lpthread

# 递归展开变量（可能有问题）
VAR = $(OTHER)
OTHER = later

# 简单展开变量（推荐）
VAR := $(OTHER)
OTHER := initial  # VAR 使用时 OTHER 还未定义
```

两种变量类型的区别：
- `=` 递归展开，在使用时才展开，可能导致无限递归
- `:=` 简单展开，在定义时就展开

### 使用变量

```makefile
$(CC)
${CC}
```

### 内置变量

```makefile
CC      # C 编译器，默认 cc
CXX     # C++ 编译器，默认 g++
CFLAGS  # C 编译选项
CXXFLAGS # C++ 编译选项
LDFLAGS # 链接选项
LDLIBS  # 链接库
AR      # 静态库创建工具
```

### 追加变量

```makefile
CFLAGS = -Wall
CFLAGS += -g
CFLAGS += -DDEBUG
```

### 条件赋值

```makefile
CC ?= gcc  # 如果 CC 未定义，则设为 gcc
```

### 命令行变量

```bash
make CC=clang CFLAGS="-O3"
```

## 常用函数

### wildcard

展开通配符，获取文件列表：

```makefile
# 获取所有 .c 文件
SRCS = $(wildcard *.c)
# SRCS = main.c utils.c debug.c

# 获取多个目录下的 .c 文件
SRCS = $(wildcard src/*.c lib/*.c)
```

### patsubst

模式替换：

```makefile
# 将 .c 文件替换为 .o 文件
SRCS = main.c utils.c
OBJS = $(patsubst %.c, %.o, $(SRCS))
# OBJS = main.o utils.o

# 更常用的写法
OBJS = $(SRCS:.c=.o)
```

### subst

简单文本替换：

```makefile
TEXT = hello world
NEW = $(subst world, make, $(TEXT))
# NEW = hello make
```

### shell

执行 shell 命令：

```makefile
DATE = $(shell date)
FILES = $(shell ls *.c)
```

### dir / notdir

提取目录或文件名：

```makefile
path = src/utils/helper.c
dir = $(dir $(path))      # src/utils/
file = $(notdir $(path))  # helper.c
```

### basename / suffix

提取基本名或后缀：

```makefile
path = helper.c
base = $(basename $(path))  # helper
ext = $(suffix $(path))     # .c
```

### addprefix / addsuffix

添加前缀或后缀：

```makefile
files = main utils
objs = $(addsuffix .o, $(files))       # main.o utils.o
paths = $(addprefix src/, $(files))    # src/main src/utils
```

## 完整示例

### 基本项目结构

```
project/
├── src/
│   ├── main.c
│   ├── utils.c
│   └── utils.h
├── include/
│   └── config.h
├── obj/
└── Makefile
```

### Makefile

```makefile
# 编译器和选项
CC := gcc
CFLAGS := -Wall -g -O2 -I./include
LDFLAGS :=

# 目录
SRC_DIR := src
OBJ_DIR := obj
INC_DIR := include
BIN_DIR := .

# 文件列表
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET := $(BIN_DIR)/program

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# 编译（带目录创建）
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 头文件依赖
$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@$(CC) -MM $(CFLAGS) $< > $@.temp
	@sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g' $@.temp > $@
	@rm -f $@.temp

# 包含依赖文件
-include $(OBJS:.o=.d)

# 清理
.PHONY: clean all

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# 调试目标
.PHONY: debug
debug:
	@echo "SRCS: $(SRCS)"
	@echo "OBJS: $(OBJS)"
	@echo "TARGET: $(TARGET)"
```

### 头文件依赖

自动检测头文件依赖变化：

```makefile
# 生成依赖文件
%.d: %.c
	$(CC) -MM $< > $@

# 包含依赖文件（如果存在）
-include $(SRCS:.c=.d)
```

`-MM` 生成适合 make 使用的依赖关系：

```bash
$ gcc -MM main.c
main.o: main.c utils.h config.h
```

## 条件判断

```makefile
# 根据变量选择不同的编译选项
DEBUG ?= 0

ifeq ($(DEBUG), 1)
    CFLAGS += -g -DDEBUG
else
    CFLAGS += -O2
endif

# 判断操作系统
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
    LDFLAGS += -lrt
endif
ifeq ($(UNAME_S), Darwin)
    LDFLAGS += -framework CoreFoundation
endif

# 判断变量是否定义
ifdef VERBOSE
    Q :=
else
    Q := @
endif

# 使用 $(Q) 控制是否显示命令
%.o: %.c
	$(Q)$(CC) $(CFLAGS) -c $< -o $@
```

## 常见技巧

### 静默运行

```makefile
# @ 开头的命令不显示命令本身
clean:
	@rm -f *.o

# 或用 - 忽略命令错误
clean:
	-rm -f *.o
```

### 命令组合

```makefile
install:
	mkdir -p /usr/local/bin && \
	cp program /usr/local/bin/ && \
	chmod +x /usr/local/bin/program
```

一行中的多个命令用 `;` 或 `&&` 连接。`\` 用于续行。

### 打印信息

```makefile
$(TARGET):
	@echo "Building $(TARGET)..."
	$(CC) $^ -o $@
	@echo "Build complete!"
```

### 并行编译

```bash
make -j4  # 使用 4 个并行任务
make -j   # 使用尽可能多的并行任务
```

在 Makefile 中设置：

```makefile
MAKEFLAGS += -j4
```

### 递归 make

对于多目录项目：

```makefile
SUBDIRS = src lib tests

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
```

## 调试 Makefile

```bash
# 显示执行的命令但不执行
make -n

# 显示变量值
make print-VAR

# 在 Makefile 中
print-%:
	@echo $* = $($*)
```

```bash
$ make print-CFLAGS
CFLAGS = -Wall -g -O2 -I./include
```
