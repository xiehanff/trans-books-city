# M02 - 动态字符串 StringBuilder

C 语言没有内置的动态字符串类型，每次拼接都需要手动管理内存。StringBuilder 通过封装字符数组、自动扩容机制，实现高效的字符串构建。

## 设计目标

- **自动扩容**：追加时自动检测容量，不足则扩容
- **高效拼接**：减少 realloc 调用次数，批量操作更高效
- **接口简洁**：链式调用风格，使用直观

## 数据结构设计

```c
typedef struct {
    char *data;      // 字符串数据（始终以 \0 结尾）
    size_t length;   // 当前长度（不含 \0）
    size_t capacity; // 总容量（含 \0）
} string_builder;
```

三个字段各司其职：

- `data`：实际存储区，堆上分配
- `length`：已使用字节数，不包含末尾 `\0`
- `capacity`：总分配空间，包含 `\0` 的位置

## API 设计

```c
// 创建与销毁
string_builder *sb_new(size_t initial_capacity);
void sb_free(string_builder *sb);

// 核心操作
void sb_append(string_builder *sb, const char *str);
void sb_append_len(string_builder *sb, const char *str, size_t len);
void sb_append_char(string_builder *sb, char c);
void sb_append_fmt(string_builder *sb, const char *fmt, ...);

// 辅助操作
void sb_clear(string_builder *sb);
const char *sb_str(string_builder *sb);
size_t sb_len(string_builder *sb);
size_t sb_capacity(string_builder *sb);

// 预分配（可选）
void sb_reserve(string_builder *sb, size_t new_capacity);
```

## 核心实现

### 扩容策略

扩容时机：`length + append_len + 1 > capacity` 时触发。

扩容策略选择：

| 策略 | 优点 | 缺点 |
|------|------|------|
| 指数扩容（x2） | 摊还复杂度 O(1)，适合大量拼接 | 可能浪费空间 |
| 固定增量 | 空间利用率高 | 频繁 realloc |
| 指数 + 阈值 | 兼顾两者 | 实现稍复杂 |

本实现采用**指数扩容 + 上限阈值**策略，初始 64 字节，每次翻倍，上限 1MB 后改为线性增长。

```c
#define SB_INITIAL_CAPACITY 64
#define SB_MAX_GROWTH       (1024 * 1024)  // 1MB

static size_t next_capacity(size_t current, size_t required) {
    size_t new_cap = current == 0 ? SB_INITIAL_CAPACITY : current * 2;
    // 线性增长阶段
    if (new_cap > SB_MAX_GROWTH) {
        new_cap = current + SB_MAX_GROWTH;
    }
    // 确保足够
    if (new_cap < required) {
        new_cap = required;
    }
    return new_cap;
}
```

### 追加实现

```c
void sb_append(string_builder *sb, const char *str) {
    if (!sb || !str) return;
    sb_append_len(sb, str, strlen(str));
}

void sb_append_len(string_builder *sb, const char *str, size_t len) {
    if (!sb || !str || len == 0) return;

    // 检查是否需要扩容（需要 +1 给 \0）
    if (sb->length + len + 1 > sb->capacity) {
        sb_reserve(sb, sb->length + len + 1);
    }

    // 追加数据
    memcpy(sb->data + sb->length, str, len);
    sb->length += len;
    sb->data[sb->length] = '\0';  // 保持 \0 结尾
}
```

`sb_append_len` 比 `sb_append` 更高效：避免内部调用 `strlen`，适合已知长度的场景（如从其他 buffer 复制）。

### 格式化追加

使用 `vsnprintf` 实现安全的格式化追加：

```c
void sb_append_fmt(string_builder *sb, const char *fmt, ...) {
    if (!sb || !fmt) return;

    va_list args;
    va_list args_copy;

    va_start(args, fmt);
    va_copy(args_copy, args);

    // 先计算需要的长度
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed < 0) {
        va_end(args_copy);
        return;
    }

    // 扩容并写入
    size_t len = (size_t)needed;
    if (sb->length + len + 1 > sb->capacity) {
        sb_reserve(sb, sb->length + len + 1);
    }

    vsnprintf(sb->data + sb->length, len + 1, fmt, args_copy);
    va_end(args_copy);

    sb->length += len;
}
```

## 完整代码实现

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define SB_INITIAL_CAPACITY 64
#define SB_MAX_GROWTH       (1024 * 1024)

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} string_builder;

// 计算下一个容量
static size_t calc_next_capacity(size_t current, size_t required) {
    size_t new_cap = current == 0 ? SB_INITIAL_CAPACITY : current * 2;

    if (new_cap > SB_MAX_GROWTH) {
        new_cap = current + SB_MAX_GROWTH;
    }
    if (new_cap < required) {
        new_cap = required;
    }
    return new_cap;
}

// 预分配容量
void sb_reserve(string_builder *sb, size_t new_capacity) {
    if (!sb || new_capacity <= sb->capacity) return;

    char *new_data = realloc(sb->data, new_capacity);
    if (!new_data) {
        // 分配失败，保持原状
        return;
    }

    sb->data = new_data;
    sb->capacity = new_capacity;

    // 首次分配时初始化为空串
    if (sb->length == 0 && sb->data) {
        sb->data[0] = '\0';
    }
}

// 创建 StringBuilder
string_builder *sb_new(size_t initial_capacity) {
    string_builder *sb = malloc(sizeof(string_builder));
    if (!sb) return NULL;

    sb->length = 0;
    sb->capacity = 0;
    sb->data = NULL;

    if (initial_capacity > 0) {
        sb_reserve(sb, initial_capacity);
    } else {
        sb_reserve(sb, SB_INITIAL_CAPACITY);
    }

    if (!sb->data) {
        free(sb);
        return NULL;
    }

    sb->data[0] = '\0';
    return sb;
}

// 销毁 StringBuilder
void sb_free(string_builder *sb) {
    if (!sb) return;
    free(sb->data);
    free(sb);
}

// 追加字符串（带长度）
void sb_append_len(string_builder *sb, const char *str, size_t len) {
    if (!sb || !str || len == 0) return;

    if (sb->length + len + 1 > sb->capacity) {
        size_t new_cap = calc_next_capacity(sb->capacity, sb->length + len + 1);
        sb_reserve(sb, new_cap);
        if (!sb->data) return;  // 分配失败
    }

    memcpy(sb->data + sb->length, str, len);
    sb->length += len;
    sb->data[sb->length] = '\0';
}

// 追加字符串
void sb_append(string_builder *sb, const char *str) {
    if (!sb || !str) return;
    sb_append_len(sb, str, strlen(str));
}

// 追加单个字符
void sb_append_char(string_builder *sb, char c) {
    if (!sb) return;
    sb_append_len(sb, &c, 1);
}

// 格式化追加
void sb_append_fmt(string_builder *sb, const char *fmt, ...) {
    if (!sb || !fmt) return;

    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);

    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed < 0) {
        va_end(args_copy);
        return;
    }

    size_t len = (size_t)needed;
    if (sb->length + len + 1 > sb->capacity) {
        size_t new_cap = calc_next_capacity(sb->capacity, sb->length + len + 1);
        sb_reserve(sb, new_cap);
        if (!sb->data) {
            va_end(args_copy);
            return;
        }
    }

    vsnprintf(sb->data + sb->length, len + 1, fmt, args_copy);
    va_end(args_copy);
    sb->length += len;
}

// 清空（保留容量）
void sb_clear(string_builder *sb) {
    if (!sb) return;
    sb->length = 0;
    if (sb->data) {
        sb->data[0] = '\0';
    }
}

// 获取 C 字符串
const char *sb_str(string_builder *sb) {
    if (!sb || !sb->data) return "";
    return sb->data;
}

// 获取长度
size_t sb_len(string_builder *sb) {
    return sb ? sb->length : 0;
}

// 获取容量
size_t sb_capacity(string_builder *sb) {
    return sb ? sb->capacity : 0;
}
```

## 使用示例

```c
int main(void) {
    // 创建 StringBuilder
    string_builder *sb = sb_new(64);

    // 基本追加
    sb_append(sb, "Hello, ");
    sb_append(sb, "World!");
    printf("%s\n", sb_str(sb));  // Hello, World!

    // 格式化追加
    sb_append_fmt(sb, " Count: %d", 42);
    printf("%s\n", sb_str(sb));  // Hello, World! Count: 42

    // 追加单个字符
    sb_append_char(sb, '\n');
    sb_append(sb, "New line");

    // 追加带长度的字符串（避免内部 strlen）
    sb_append_len(sb, "[END]", 5);

    // 清空重用
    sb_clear(sb);
    printf("After clear: '%s' (len=%zu)\n", sb_str(sb), sb_len(sb));

    // 大量拼接测试
    for (int i = 0; i < 1000; i++) {
        sb_append_fmt(sb, "item_%d,", i);
    }
    printf("Large string len: %zu, capacity: %zu\n",
           sb_len(sb), sb_capacity(sb));

    sb_free(sb);
    return 0;
}
```

## 性能优化思路

### 预分配合理容量

如果知道大致的最终长度，提前 `sb_reserve` 可避免多次扩容：

```c
string_builder *sb = sb_new(0);
sb_reserve(sb, 4096);  // 预分配 4KB
// 后续追加不会触发扩容
```

### 批量追加 vs 单次追加

```c
// 低效：多次调用
sb_append(sb, "a");
sb_append(sb, "b");
sb_append(sb, "c");

// 高效：一次调用
sb_append(sb, "abc");
```

### 避免频繁调用 strlen

`sb_append_len` 比 `sb_append` 省掉一次 `strlen` 调用，在循环中差异明显：

```c
// 假设从某个 buffer 读取数据
char buffer[1024];
size_t n = read(fd, buffer, sizeof(buffer));

// 高效：已知长度
sb_append_len(sb, buffer, n);

// 低效：需要额外计算长度
sb_append(sb, buffer);
```

## 进阶：链式调用

返回 `string_builder*` 可以实现链式 API：

```c
string_builder *sb_append_chain(string_builder *sb, const char *str) {
    sb_append(sb, str);
    return sb;
}

// 使用
sb_append_chain(sb_append_chain(sb, "Hello, "), "World!");
```

考虑到 C 语言的错误处理习惯，链式调用会增加返回值判断的复杂度。本实现选择 `void` 返回，保持接口简单。

## 扩展方向

1. **插入/删除操作**：支持在任意位置插入或删除子串
2. **子串查找**：实现 `sb_find`、`sb_rfind` 等搜索函数
3. **修剪功能**：`sb_trim` 去除首尾空白
4. **编码支持**：UTF-8 感知的字符计数和截断
5. **写入文件**：`sb_write_file` 直接输出到文件

这些扩展会增加代码复杂度，按需实现即可。
