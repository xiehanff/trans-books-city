# M01 - 轻量级 JSON 解析器

设计一个无第三方依赖、单文件实现的 JSON 解析器，支持完整的 JSON 数据类型。

## 设计目标

- 零依赖，单文件实现
- 支持全部 JSON 类型：null、boolean、number、string、array、object
- 内存可追踪释放
- API 简洁直观

## 数据结构设计

```c
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// JSON 值类型枚举
typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} json_type;

// JSON 值结构体
typedef struct json_value {
    json_type type;
    union {
        int boolean;                                    // 布尔值
        double number;                                  // 数值
        char *string;                                   // 字符串
        struct {
            struct json_value *items;                   // 数组元素
            size_t count;                               // 元素数量
        } array;
        struct {
            char **keys;                                // 对象键名
            struct json_value *values;                  // 对象值
            size_t count;                               // 成员数量
        } object;
    } data;
} json_value;
```

联合体存储不同类型的数据，通过 `type` 字段判断当前类型。数组和对象采用动态数组存储。

## 解析上下文

```c
// 解析上下文，跟踪当前位置
typedef struct {
    const char *json;      // JSON 字符串指针
    size_t pos;            // 当前解析位置
    size_t len;            // 字符串总长度
} json_context;

// 跳过空白字符
static void skip_whitespace(json_context *ctx) {
    while (ctx->pos < ctx->len) {
        char c = ctx->json[ctx->pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            ctx->pos++;
        } else {
            break;
        }
    }
}

// 获取当前字符
static char current_char(json_context *ctx) {
    return (ctx->pos < ctx->len) ? ctx->json[ctx->pos] : '\0';
}

// 向前移动
static void advance(json_context *ctx) {
    if (ctx->pos < ctx->len) {
        ctx->pos++;
    }
}

static void json_destroy_contents(json_value *val);

static void json_destroy_array_items(json_value *items, size_t count) {
    if (!items) return;
    for (size_t i = 0; i < count; i++) {
        json_destroy_contents(&items[i]);
    }
    free(items);
}

static void json_destroy_object_items(char **keys, json_value *values, size_t count) {
    if (!keys || !values) {
        free(keys);
        free(values);
        return;
    }

    for (size_t i = 0; i < count; i++) {
        free(keys[i]);
        json_destroy_contents(&values[i]);
    }
    free(keys);
    free(values);
}

static void json_destroy_contents(json_value *val) {
    if (!val) return;

    switch (val->type) {
        case JSON_STRING:
            free(val->data.string);
            break;
        case JSON_ARRAY:
            json_destroy_array_items(val->data.array.items, val->data.array.count);
            break;
        case JSON_OBJECT:
            json_destroy_object_items(val->data.object.keys, val->data.object.values, val->data.object.count);
            break;
        default:
            break;
    }

    val->type = JSON_NULL;
}
```

## 核心解析函数

### null 解析

```c
static int parse_null(json_context *ctx, json_value *val) {
    if (ctx->pos + 4 > ctx->len) return 0;
    if (strncmp(ctx->json + ctx->pos, "null", 4) != 0) return 0;

    val->type = JSON_NULL;
    ctx->pos += 4;
    return 1;
}
```

### boolean 解析

```c
static int parse_bool(json_context *ctx, json_value *val) {
    val->type = JSON_BOOL;

    if (ctx->pos + 4 <= ctx->len &&
        strncmp(ctx->json + ctx->pos, "true", 4) == 0) {
        val->data.boolean = 1;
        ctx->pos += 4;
        return 1;
    }

    if (ctx->pos + 5 <= ctx->len &&
        strncmp(ctx->json + ctx->pos, "false", 5) == 0) {
        val->data.boolean = 0;
        ctx->pos += 5;
        return 1;
    }

    return 0;
}
```

### number 解析

```c
static int parse_number(json_context *ctx, json_value *val) {
    size_t start = ctx->pos;

    // 负号
    if (current_char(ctx) == '-') {
        advance(ctx);
    }

    // 整数部分
    if (current_char(ctx) == '0') {
        advance(ctx);
    } else if (isdigit((unsigned char)current_char(ctx))) {
        while (isdigit((unsigned char)current_char(ctx))) {
            advance(ctx);
        }
    } else {
        return 0;
    }

    // 小数部分
    int has_frac = 0;
    if (current_char(ctx) == '.') {
        has_frac = 1;
        advance(ctx);
        if (!isdigit((unsigned char)current_char(ctx))) return 0;
        while (isdigit((unsigned char)current_char(ctx))) {
            advance(ctx);
        }
    }

    // 指数部分
    if (current_char(ctx) == 'e' || current_char(ctx) == 'E') {
        advance(ctx);
        if (current_char(ctx) == '+' || current_char(ctx) == '-') {
            advance(ctx);
        }
        if (!isdigit((unsigned char)current_char(ctx))) return 0;
        while (isdigit((unsigned char)current_char(ctx))) {
            advance(ctx);
        }
    }

    // 提取子串并转换
    size_t num_len = ctx->pos - start;
    char *num_str = (char *)malloc(num_len + 1);
    if (!num_str) return 0;

    memcpy(num_str, ctx->json + start, num_len);
    num_str[num_len] = '\0';

    val->type = JSON_NUMBER;
    val->data.number = strtod(num_str, NULL);
    free(num_str);

    return 1;
}
```

### string 解析

```c
// 解析 Unicode 转义，返回码点
static int parse_hex4(const char *p) {
    int code = 0;
    for (int i = 0; i < 4; i++) {
        code <<= 4;
        char c = p[i];
        if (c >= '0' && c <= '9') code |= (c - '0');
        else if (c >= 'a' && c <= 'f') code |= (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') code |= (c - 'A' + 10);
        else return -1;
    }
    return code;
}

// 将码点编码为 UTF-8
static int encode_utf8(int code, char *buf) {
    if (code < 0x80) {
        buf[0] = (char)code;
        return 1;
    } else if (code < 0x800) {
        buf[0] = (char)(0xC0 | (code >> 6));
        buf[1] = (char)(0x80 | (code & 0x3F));
        return 2;
    } else if (code < 0x10000) {
        buf[0] = (char)(0xE0 | (code >> 12));
        buf[1] = (char)(0x80 | ((code >> 6) & 0x3F));
        buf[2] = (char)(0x80 | (code & 0x3F));
        return 3;
    } else {
        buf[0] = (char)(0xF0 | (code >> 18));
        buf[1] = (char)(0x80 | ((code >> 12) & 0x3F));
        buf[2] = (char)(0x80 | ((code >> 6) & 0x3F));
        buf[3] = (char)(0x80 | (code & 0x3F));
        return 4;
    }
}

static int parse_string(json_context *ctx, json_value *val) {
    if (current_char(ctx) != '"') return 0;
    advance(ctx);  // 跳过开头引号

    // 第一遍：计算长度
    size_t start = ctx->pos;
    size_t out_len = 0;
    while (current_char(ctx) != '"' && current_char(ctx) != '\0') {
        if (current_char(ctx) == '\\') {
            advance(ctx);
            char esc = current_char(ctx);
            if (esc == 'u') {
                // Unicode 转义，最多 4 字节 UTF-8
                out_len += 4;
                ctx->pos += 5;
            } else {
                out_len++;
                advance(ctx);
            }
        } else {
            out_len++;
            advance(ctx);
        }
    }

    if (current_char(ctx) != '"') return 0;

    // 分配内存
    char *str = (char *)malloc(out_len + 1);
    if (!str) return 0;

    // 第二遍：实际解析
    ctx->pos = start;
    char *out = str;

    while (current_char(ctx) != '"') {
        if (current_char(ctx) == '\\') {
            advance(ctx);
            char esc = current_char(ctx);
            char mapped;
            switch (esc) {
                case '"':  mapped = '"';  break;
                case '\\': mapped = '\\'; break;
                case '/':  mapped = '/';  break;
                case 'b':  mapped = '\b'; break;
                case 'f':  mapped = '\f'; break;
                case 'n':  mapped = '\n'; break;
                case 'r':  mapped = '\r'; break;
                case 't':  mapped = '\t'; break;
                case 'u': {
                    // Unicode 转义
                    int code = parse_hex4(ctx->json + ctx->pos + 1);
                    if (code < 0) {
                        free(str);
                        return 0;
                    }
                    ctx->pos += 5;

                    // 处理代理对
                    if (code >= 0xD800 && code <= 0xDBFF) {
                        if (current_char(ctx) == '\\' && ctx->json[ctx->pos + 1] == 'u') {
                            int low = parse_hex4(ctx->json + ctx->pos + 2);
                            if (low >= 0xDC00 && low <= 0xDFFF) {
                                code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                                ctx->pos += 6;
                            }
                        }
                    }

                    out += encode_utf8(code, out);
                    continue;
                }
                default:
                    free(str);
                    return 0;
            }
            *out++ = mapped;
            advance(ctx);
        } else {
            *out++ = current_char(ctx);
            advance(ctx);
        }
    }

    *out = '\0';
    if (current_char(ctx) != '"') {
        free(str);
        return 0;
    }
    advance(ctx);  // 跳过结尾引号

    val->type = JSON_STRING;
    val->data.string = str;
    return 1;
}
```

### array 解析

```c
// 前向声明
static int parse_value(json_context *ctx, json_value *val);

static int parse_array(json_context *ctx, json_value *val) {
    if (current_char(ctx) != '[') return 0;
    advance(ctx);

    val->type = JSON_ARRAY;
    val->data.array.items = NULL;
    val->data.array.count = 0;

    skip_whitespace(ctx);
    if (current_char(ctx) == ']') {
        advance(ctx);
        return 1;  // 空数组
    }

    // 动态扩容
    size_t capacity = 4;
    val->data.array.items = (json_value *)malloc(capacity * sizeof(json_value));
    if (!val->data.array.items) return 0;

    while (1) {
        skip_whitespace(ctx);

        // 扩容检查
        if (val->data.array.count >= capacity) {
            capacity *= 2;
            json_value *new_items = (json_value *)realloc(
                val->data.array.items, capacity * sizeof(json_value));
            if (!new_items) {
                json_destroy_array_items(val->data.array.items, val->data.array.count);
                return 0;
            }
            val->data.array.items = new_items;
        }

        // 解析元素
        json_value *item = &val->data.array.items[val->data.array.count];
        if (!parse_value(ctx, item)) {
            json_destroy_array_items(val->data.array.items, val->data.array.count);
            return 0;
        }
        val->data.array.count++;

        skip_whitespace(ctx);
        if (current_char(ctx) == ',') {
            advance(ctx);
        } else if (current_char(ctx) == ']') {
            advance(ctx);
            break;
        } else {
            json_destroy_array_items(val->data.array.items, val->data.array.count);
            return 0;
        }
    }

    return 1;
}
```

### object 解析

```c
static int parse_object(json_context *ctx, json_value *val) {
    if (current_char(ctx) != '{') return 0;
    advance(ctx);

    val->type = JSON_OBJECT;
    val->data.object.keys = NULL;
    val->data.object.values = NULL;
    val->data.object.count = 0;

    skip_whitespace(ctx);
    if (current_char(ctx) == '}') {
        advance(ctx);
        return 1;  // 空对象
    }

    size_t capacity = 4;
    val->data.object.keys = (char **)malloc(capacity * sizeof(char *));
    val->data.object.values = (json_value *)malloc(capacity * sizeof(json_value));
    if (!val->data.object.keys || !val->data.object.values) {
        free(val->data.object.keys);
        free(val->data.object.values);
        return 0;
    }

    while (1) {
        skip_whitespace(ctx);

        // 扩容
        if (val->data.object.count >= capacity) {
            size_t new_capacity = capacity * 2;
            char **new_keys = (char **)malloc(new_capacity * sizeof(char *));
            json_value *new_values = (json_value *)malloc(new_capacity * sizeof(json_value));
            if (!new_keys || !new_values) {
                free(new_keys);
                free(new_values);
                json_destroy_object_items(val->data.object.keys, val->data.object.values, val->data.object.count);
                return 0;
            }
            memcpy(new_keys, val->data.object.keys, val->data.object.count * sizeof(char *));
            memcpy(new_values, val->data.object.values, val->data.object.count * sizeof(json_value));
            free(val->data.object.keys);
            free(val->data.object.values);
            val->data.object.keys = new_keys;
            val->data.object.values = new_values;
            capacity = new_capacity;
        }

        // 解析键
        size_t index = val->data.object.count;
        json_value key_val;
        if (!parse_string(ctx, &key_val)) {
            json_destroy_object_items(val->data.object.keys, val->data.object.values, val->data.object.count);
            return 0;
        }
        val->data.object.keys[index] = key_val.data.string;

        skip_whitespace(ctx);
        if (current_char(ctx) != ':') {
            free(val->data.object.keys[index]);
            json_destroy_object_items(val->data.object.keys, val->data.object.values, val->data.object.count);
            return 0;
        }
        advance(ctx);

        skip_whitespace(ctx);

        // 解析值
        if (!parse_value(ctx, &val->data.object.values[index])) {
            free(val->data.object.keys[index]);
            json_destroy_object_items(val->data.object.keys, val->data.object.values, val->data.object.count);
            return 0;
        }
        val->data.object.count++;

        skip_whitespace(ctx);
        if (current_char(ctx) == ',') {
            advance(ctx);
        } else if (current_char(ctx) == '}') {
            advance(ctx);
            break;
        } else {
            json_destroy_object_items(val->data.object.keys, val->data.object.values, val->data.object.count);
            return 0;
        }
    }

    return 1;
}
```

### 解析入口

```c
static int parse_value(json_context *ctx, json_value *val) {
    skip_whitespace(ctx);

    char c = current_char(ctx);
    int result = 0;

    switch (c) {
        case 'n':
            result = parse_null(ctx, val);
            break;
        case 't':
        case 'f':
            result = parse_bool(ctx, val);
            break;
        case '"':
            result = parse_string(ctx, val);
            break;
        case '[':
            result = parse_array(ctx, val);
            break;
        case '{':
            result = parse_object(ctx, val);
            break;
        default:
            if (c == '-' || isdigit((unsigned char)c)) {
                result = parse_number(ctx, val);
            }
            break;
    }

    return result;
}

// 主解析入口
json_value *json_parse(const char *input) {
    if (!input) return NULL;

    json_value *val = (json_value *)malloc(sizeof(json_value));
    if (!val) return NULL;

    json_context ctx = { input, 0, strlen(input) };

    if (!parse_value(&ctx, val)) {
        free(val);
        return NULL;
    }

    skip_whitespace(&ctx);
    if (current_char(&ctx) != '\0') {
        // 有多余字符，解析失败
        json_destroy_contents(val);
        free(val);
        return NULL;
    }

    return val;
}
```

## 内存释放

```c
void json_free(json_value *val) {
    if (!val) return;
    json_destroy_contents(val);
    free(val);
}
```

## 访问接口

```c
json_type json_get_type(json_value *val) {
    return val ? val->type : JSON_NULL;
}

int json_is_null(json_value *val) {
    return val && val->type == JSON_NULL;
}

int json_is_bool(json_value *val) {
    return val && val->type == JSON_BOOL;
}

int json_is_number(json_value *val) {
    return val && val->type == JSON_NUMBER;
}

int json_is_string(json_value *val) {
    return val && val->type == JSON_STRING;
}

int json_is_array(json_value *val) {
    return val && val->type == JSON_ARRAY;
}

int json_is_object(json_value *val) {
    return val && val->type == JSON_OBJECT;
}

int json_get_bool(json_value *val) {
    return (val && val->type == JSON_BOOL) ? val->data.boolean : 0;
}

double json_get_number(json_value *val) {
    return (val && val->type == JSON_NUMBER) ? val->data.number : 0.0;
}

const char *json_get_string(json_value *val) {
    return (val && val->type == JSON_STRING) ? val->data.string : NULL;
}

size_t json_array_size(json_value *arr) {
    return (arr && arr->type == JSON_ARRAY) ? arr->data.array.count : 0;
}

json_value *json_array_get(json_value *arr, size_t index) {
    if (!arr || arr->type != JSON_ARRAY || index >= arr->data.array.count) {
        return NULL;
    }
    return &arr->data.array.items[index];
}

size_t json_object_size(json_value *obj) {
    return (obj && obj->type == JSON_OBJECT) ? obj->data.object.count : 0;
}

json_value *json_object_get(json_value *obj, const char *key) {
    if (!obj || obj->type != JSON_OBJECT || !key) return NULL;

    for (size_t i = 0; i < obj->data.object.count; i++) {
        if (strcmp(obj->data.object.keys[i], key) == 0) {
            return &obj->data.object.values[i];
        }
    }
    return NULL;
}

// 遍历对象：获取第 i 个键值对
const char *json_object_key(json_value *obj, size_t index) {
    if (!obj || obj->type != JSON_OBJECT || index >= obj->data.object.count) {
        return NULL;
    }
    return obj->data.object.keys[index];
}

json_value *json_object_value(json_value *obj, size_t index) {
    if (!obj || obj->type != JSON_OBJECT || index >= obj->data.object.count) {
        return NULL;
    }
    return &obj->data.object.values[index];
}
```

## 完整代码

将以上片段合并为单文件 `json_parser.h`：

```c
#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    JSON_NULL, JSON_BOOL, JSON_NUMBER, JSON_STRING, JSON_ARRAY, JSON_OBJECT
} json_type;

typedef struct json_value {
    json_type type;
    union {
        int boolean;
        double number;
        char *string;
        struct { struct json_value *items; size_t count; } array;
        struct { char **keys; struct json_value *values; size_t count; } object;
    } data;
} json_value;

// 解析与释放
json_value *json_parse(const char *input);
void json_free(json_value *val);

// 类型判断
json_type json_get_type(json_value *val);
int json_is_null(json_value *val);
int json_is_bool(json_value *val);
int json_is_number(json_value *val);
int json_is_string(json_value *val);
int json_is_array(json_value *val);
int json_is_object(json_value *val);

// 值获取
int json_get_bool(json_value *val);
double json_get_number(json_value *val);
const char *json_get_string(json_value *val);

// 数组操作
size_t json_array_size(json_value *arr);
json_value *json_array_get(json_value *arr, size_t index);

// 对象操作
size_t json_object_size(json_value *obj);
json_value *json_object_get(json_value *obj, const char *key);
const char *json_object_key(json_value *obj, size_t index);
json_value *json_object_value(json_value *obj, size_t index);

#ifdef __cplusplus
}
#endif

#endif // JSON_PARSER_H
```

实现代码放入 `json_parser.c`，包含所有静态解析函数和导出函数的实现。

## 使用示例

```c
#include <stdio.h>
#include "json_parser.h"

int main(void) {
    const char *json_text = "{"
        "\"name\": \"test\","
        "\"value\": 123.45,"
        "\"active\": true,"
        "\"tags\": [\"a\", \"b\", \"c\"],"
        "\"nested\": {\"key\": \"inner\"}"
    "}";

    json_value *root = json_parse(json_text);
    if (!root) {
        printf("解析失败\n");
        return 1;
    }

    // 读取字符串
    const char *name = json_get_string(json_object_get(root, "name"));
    printf("name: %s\n", name);

    // 读取数值
    double value = json_get_number(json_object_get(root, "value"));
    printf("value: %g\n", value);

    // 读取布尔值
    int active = json_get_bool(json_object_get(root, "active"));
    printf("active: %s\n", active ? "true" : "false");

    // 遍历数组
    json_value *tags = json_object_get(root, "tags");
    for (size_t i = 0; i < json_array_size(tags); i++) {
        printf("tags[%zu]: %s\n", i, json_get_string(json_array_get(tags, i)));
    }

    // 读取嵌套对象
    json_value *nested = json_object_get(root, "nested");
    const char *inner = json_get_string(json_object_get(nested, "key"));
    printf("nested.key: %s\n", inner);

    json_free(root);
    return 0;
}
```

输出：

```
name: test
value: 123.45
active: true
tags[0]: a
tags[1]: b
tags[2]: c
nested.key: inner
```

## 性能优化思路

**减少内存分配**

当前方案对每个字符串、数组、对象都要动态分配。可以采用"一次分配"策略：解析前先扫描整个 JSON，统计需要的总内存，然后一次性分配大块内存，后续所有子结构都从这块内存中切分。

**原地解析**

如果允许修改输入字符串，可以不复制字符串内容，直接在原字符串上做跳转。字符串值存储指针而非副本，节省大量分配和复制开销。

```c
// 原地解析的字符串结构
struct {
    const char *ptr;    // 指向原字符串
    size_t len;         // 长度（转义已处理）
};
```

**SIMD 加速**

跳过空白字符占用可观时间。使用 SSE/AVX 指令可以一次处理 16/32 字节，快速找到下一个非空白位置：

```c
#include <immintrin.h>

static size_t skip_whitespace_simd(const char *p, size_t len) {
    size_t i = 0;
    // 每次处理 32 字节
    for (; i + 32 <= len; i += 32) {
        __m256i chunk = _mm256_loadu_si256((__m256i *)(p + i));
        // 构造空白字符掩码
        __m256i sp = _mm256_set1_epi8(' ');
        __m256i tab = _mm256_set1_epi8('\t');
        __m256i nl = _mm256_set1_epi8('\n');
        __m256i cr = _mm256_set1_epi8('\r');
        // 比较并合并结果...
    }
    // 处理剩余字节
    for (; i < len; i++) {
        char c = p[i];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
    }
    return i;
}
```

**哈希表优化对象查找**

当前 `json_object_get` 是线性查找，O(n) 复杂度。对于成员较多的对象，可以用开放寻址哈希表存储键值对，将查找降到 O(1)。

## 错误处理增强

生产环境需要更详细的错误信息：

```c
typedef struct {
    int code;           // 错误码
    size_t position;    // 错误位置
    const char *message;
} json_error;

json_value *json_parse_ex(const char *input, json_error *error);
```

解析失败时填充 `error` 结构，便于定位问题。

## 小结

这个解析器约 400 行代码，实现了完整的 JSON 解析功能。核心思路是递归下降解析：每个类型对应一个解析函数，通过首字符判断进入哪个分支。内存管理采用逐层释放，释放顺序与分配顺序相反。

实际项目中可按需剪裁：只解析特定类型、只读不写、添加格式化输出等。完整实现可以参考 cJSON、yj2l 等开源项目。
