# 内存池 Memory Pool

内存池通过预分配大块内存，从中按需切分小块供程序使用，减少频繁调用 malloc/free 带来的开销和碎片。

## 为什么需要内存池

**减少分配器开销**

`malloc` 和 `free` 会经过分配器管理逻辑。高频分配释放时，这部分开销会累积起来；某些情况下分配器还会向操作系统申请或归还内存，但这不是每次调用都会发生。

**避免内存碎片**

反复分配释放不同大小的块，会导致堆内存变得支离破碎。即使总空闲空间足够，也可能无法分配大块连续内存。

**简化生命周期管理**

游戏一局结束、服务器请求处理完毕后，可以直接销毁整个内存池，不必逐个释放对象。

**典型场景对比**

| 场景 | malloc/free | 内存池 |
|------|-------------|--------|
| 游戏：每帧创建/销毁粒子 | 频繁调用，管理开销较高 | 统一复用预分配内存 |
| 服务器：每请求分配上下文 | 需要反复申请和释放 | 可按请求生命周期批量回收 |
| 编译器：构建 AST | 递归释放代码较繁琐 | arena 结束后一次性重置 |

## 设计方案

### 方案一：固定大小块池

每个块大小固定，用空闲链表管理已释放的块。

```
初始状态：
+---+---+---+---+---+
| 0 | 1 | 2 | 3 | 4 |   ← 预分配的大块内存
+---+---+---+---+---+

空闲链表：0 → 1 → 2 → 3 → 4 → NULL

分配后：
+---+---+---+---+---+
| X | X | 2 | 3 | 4 |   ← X 表示已分配
+---+---+---+---+---+

空闲链表：2 → 3 → 4 → NULL

释放块 1 后：
+---+---+---+---+---+
| X | 1 | 2 | 3 | 4 |   ← 块 1 回到链表头部
+---+---+---+---+---+

空闲链表：1 → 2 → 3 → 4 → NULL
```

**时间复杂度：O(1) 分配和释放**

```c
typedef struct mempool {
    size_t block_size;    // 每个块的大小
    size_t capacity;      // 总块数
    char *memory;         // 预分配的大块内存
    void **free_list;     // 空闲块指针数组
    size_t free_count;    // 当前空闲块数
} mempool;

mempool *mempool_create(size_t block_size, size_t count);
void *mempool_alloc(mempool *pool);
void mempool_free(mempool *pool, void *ptr);
void mempool_destroy(mempool *pool);
```

### 方案二：可变大小池（Arena）

线性分配，只增不减。不支持单独释放，只能整体重置。

```
初始状态：
+-------------------------------------------+
|                                           |
+-------------------------------------------+
 ↑ offset = 0

分配 16 字节：
+----------------+--------------------------+
|     16B        |                          |
+----------------+--------------------------+
 ↑ offset = 0    ↑ offset = 16

分配 32 字节：
+----------------+--------------------+------+
|     16B        |       32B          |      |
+----------------+--------------------+------+
                  ↑ offset = 48

reset 后：
+-------------------------------------------+
|                                           |
+-------------------------------------------+
 ↑ offset = 0  （所有指针失效）
```

**适用场景：生命周期一致的批量分配，如 AST 构建请求上下文**

```c
typedef struct arena {
    char *buffer;      // 预分配内存
    size_t capacity;   // 总容量
    size_t offset;     // 当前分配位置
} arena;

arena *arena_create(size_t capacity);
void *arena_alloc(arena *a, size_t size);
void arena_reset(arena *a);   // 重置，所有指针失效
void arena_destroy(arena *a);
```

## 完整代码实现

固定大小块池的完整实现：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 内存对齐：大多数系统上 16 字节对齐足够
#define ALIGN_SIZE 16
#define ALIGN_UP(x) (((x) + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))

typedef struct mempool {
    size_t block_size;    // 对齐后的块大小
    size_t capacity;      // 总块数
    char *memory;         // 预分配的大块内存
    void **free_list;     // 空闲块指针数组
    size_t free_count;    // 当前空闲块数
} mempool;

// 创建内存池
mempool *mempool_create(size_t block_size, size_t count) {
    if (block_size == 0 || count == 0) {
        return NULL;
    }

    // 分配池结构体
    mempool *pool = (mempool *)malloc(sizeof(mempool));
    if (!pool) return NULL;

    // 对齐块大小
    pool->block_size = ALIGN_UP(block_size);
    pool->capacity = count;

    // 分配实际内存块
    pool->memory = (char *)malloc(pool->block_size * count);
    if (!pool->memory) {
        free(pool);
        return NULL;
    }

    // 分配空闲链表数组
    pool->free_list = (void **)malloc(sizeof(void *) * count);
    if (!pool->free_list) {
        free(pool->memory);
        free(pool);
        return NULL;
    }

    // 初始化：所有块都是空闲的，加入链表
    for (size_t i = 0; i < count; i++) {
        pool->free_list[i] = pool->memory + i * pool->block_size;
    }
    pool->free_count = count;

    return pool;
}

// 从池中分配一个块
void *mempool_alloc(mempool *pool) {
    if (!pool || pool->free_count == 0) {
        return NULL;  // 池已空
    }

    // 从空闲链表头部取一个块
    pool->free_count--;
    void *ptr = pool->free_list[pool->free_count];

    return ptr;
}

// 将块归还到池中
void mempool_free(mempool *pool, void *ptr) {
    if (!pool || !ptr) return;

    // 检查指针是否属于这个池
    char *p = (char *)ptr;
    if (p < pool->memory ||
        p >= pool->memory + pool->block_size * pool->capacity) {
        fprintf(stderr, "警告：指针 %p 不属于此内存池\n", ptr);
        return;
    }

    // 检查是否已对齐到块边界
    size_t offset = p - pool->memory;
    if (offset % pool->block_size != 0) {
        fprintf(stderr, "警告：指针 %p 未对齐到块边界\n", ptr);
        return;
    }

    if (pool->free_count >= pool->capacity) {
        fprintf(stderr, "警告：内存池已满，无法重复归还 %p\n", ptr);
        return;
    }

    // 归还到空闲链表
    pool->free_list[pool->free_count] = ptr;
    pool->free_count++;
}

// 销毁内存池
void mempool_destroy(mempool *pool) {
    if (!pool) return;

    free(pool->free_list);
    free(pool->memory);
    free(pool);
}

// 获取池状态信息
void mempool_stats(mempool *pool) {
    if (!pool) return;
    size_t used = pool->free_count <= pool->capacity ? pool->capacity - pool->free_count : 0;
    printf("内存池状态:\n");
    printf("  块大小: %zu 字节\n", pool->block_size);
    printf("  总容量: %zu 块\n", pool->capacity);
    printf("  已使用: %zu 块\n", used);
    printf("  空闲数: %zu 块\n", pool->free_count);
    printf("  总内存: %zu 字节\n", pool->block_size * pool->capacity);
}
```

## 使用示例

```c
// 示例数据结构
typedef struct {
    int id;
    char name[32];
    double value;
} Particle;

int main(void) {
    // 创建内存池：1000 个 Particle
    mempool *pool = mempool_create(sizeof(Particle), 1000);
    if (!pool) {
        fprintf(stderr, "创建内存池失败\n");
        return 1;
    }

    // 分配粒子
    Particle *p1 = (Particle *)mempool_alloc(pool);
    Particle *p2 = (Particle *)mempool_alloc(pool);
    Particle *p3 = (Particle *)mempool_alloc(pool);

    // 使用
    if (p1) {
        p1->id = 1;
        strcpy(p1->name, "fire");
        p1->value = 100.0;
    }

    // 查看状态
    mempool_stats(pool);

    // 释放粒子
    mempool_free(pool, p1);
    mempool_free(pool, p2);

    // 销毁整个池
    mempool_destroy(pool);

    return 0;
}
```

输出：

```
内存池状态:
  块大小: 48 字节
  总容量: 1000 块
  已使用: 1 块
  空闲数: 999 块
  总内存: 48000 字节
```

## Arena 实现示例

适用于编译器 AST、请求上下文等场景：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_ALIGN 16
#define ARENA_ALIGN_UP(x) (((x) + ARENA_ALIGN - 1) & ~(ARENA_ALIGN - 1))

typedef struct arena {
    char *buffer;
    size_t capacity;
    size_t offset;
} arena;

arena *arena_create(size_t capacity) {
    arena *a = (arena *)malloc(sizeof(arena));
    if (!a) return NULL;

    // 预分配缓冲区
    a->buffer = (char *)malloc(capacity);
    if (!a->buffer) {
        free(a);
        return NULL;
    }

    a->capacity = capacity;
    a->offset = 0;
    return a;
}

void *arena_alloc(arena *a, size_t size) {
    if (!a) return NULL;

    size = ARENA_ALIGN_UP(size);

    if (a->offset + size > a->capacity) {
        return NULL;  // 空间不足
    }

    void *ptr = a->buffer + a->offset;
    a->offset += size;
    return ptr;
}

// 分配并清零
void *arena_calloc(arena *a, size_t size) {
    void *ptr = arena_alloc(a, size);
    if (ptr) {
        memset(ptr, 0, ARENA_ALIGN_UP(size));
    }
    return ptr;
}

// 重置：所有分配的指针失效
void arena_reset(arena *a) {
    if (a) {
        a->offset = 0;
    }
}

void arena_destroy(arena *a) {
    if (!a) return;
    free(a->buffer);
    free(a);
}

// 使用示例：构建简化 AST
typedef struct Node {
    int type;
    struct Node *left;
    struct Node *right;
    char value[32];
} Node;

Node *create_node(arena *a, int type, const char *value) {
    Node *n = (Node *)arena_calloc(a, sizeof(Node));
    if (n) {
        n->type = type;
        strncpy(n->value, value, sizeof(n->value) - 1);
    }
    return n;
}

int main(void) {
    arena *a = arena_create(1024 * 1024);  // 1MB

    // 构建表达式树：(a + b) * c
    Node *n1 = create_node(a, 1, "a");
    Node *n2 = create_node(a, 1, "b");
    Node *plus = create_node(a, 2, "+");
    plus->left = n1;
    plus->right = n2;

    Node *n3 = create_node(a, 1, "c");
    Node *mul = create_node(a, 2, "*");
    mul->left = plus;
    mul->right = n3;

    printf("Arena 使用: %zu 字节\n", a->offset);
    // 处理完后一键重置，无需递归释放
    arena_reset(a);
    arena_destroy(a);

    return 0;
}
```

## 典型应用场景

**游戏粒子系统**

```c
// 每帧可能创建数百个粒子，帧尾统一回收
mempool *particle_pool = mempool_create(sizeof(Particle), 5000);

void update_particles(void) {
    // 分配活跃粒子
    for (int i = 0; i < new_particle_count; i++) {
        Particle *p = mempool_alloc(particle_pool);
        init_particle(p);
    }

    // 更新并回收死亡粒子
    for (each active particle) {
        if (particle_is_dead(p)) {
            mempool_free(particle_pool, p);
        }
    }
}
```

**网络服务器连接对象**

```c
#include <time.h>

// 连接建立时分配，断开时归还
mempool *conn_pool = mempool_create(sizeof(Connection), 10000);

void on_client_connect(int fd) {
    Connection *conn = mempool_alloc(conn_pool);
    if (!conn) {
        return;
    }
    conn->fd = fd;
    conn->create_time = time(NULL);
    // ...
}

void on_client_disconnect(Connection *conn) {
    mempool_free(conn_pool, conn);
}
```

**编译器 AST 构建**

```c
// 整个编译过程用同一个 arena，编译结束一键释放
arena *ast_arena = arena_create(10 * 1024 * 1024);  // 10MB

ASTNode *parse_expression(void) {
    ASTNode *node = arena_calloc(ast_arena, sizeof(ASTNode));
    // 递归构建子节点...
    return node;
}

void compile(FILE *src) {
    ASTNode *ast = parse(src);
    generate_code(ast);
    // 无需遍历 AST 释放，直接重置
    arena_reset(ast_arena);
}
```

**HTTP 请求上下文**

```c
arena *req_arena = arena_create(64 * 1024);  // 64KB per request

void handle_request(Request *req) {
    // 所有临时分配都在 arena 上
    char *path = arena_alloc(req_arena, 256);
    Header *headers = arena_calloc(req_arena, sizeof(Header) * 64);
    Buffer *body = arena_alloc(req_arena, req->content_length);

    // 处理请求...

    // 请求结束，一键清理
    arena_reset(req_arena);
}
```

## 性能优化思路

**内存对齐**

```c
// 未对齐可能导致跨缓存行访问，性能下降
#define ALIGN 16
#define ALIGN_UP(x) (((x) + ALIGN - 1) & ~(ALIGN - 1))

// 或使用编译器内置
#define ALIGN_UP(x) __builtin_align_up(x, 16)
```

**多线程支持**

简单加锁方案：

```c
typedef struct {
    mempool pool;
    pthread_mutex_t lock;
} thread_safe_pool;

void *ts_alloc(thread_safe_pool *ts) {
    pthread_mutex_lock(&ts->lock);
    void *ptr = mempool_alloc(&ts->pool);
    pthread_mutex_unlock(&ts->lock);
    return ptr;
}
```

Per-thread 池（无锁）：

```c
// 每个线程维护独立的池，避免锁竞争
__thread mempool *local_pool = NULL;

void *local_alloc(void) {
    if (!local_pool) {
        local_pool = mempool_create(BLOCK_SIZE, LOCAL_COUNT);
    }
    return mempool_alloc(local_pool);
}
```

**内存预热点**

首次分配可能触发缺页中断，预热可提升后续性能：

```c
void mempool_warmup(mempool *pool) {
    // 写入每个块，确保物理页已分配
    for (size_t i = 0; i < pool->capacity; i++) {
        char *block = pool->memory + i * pool->block_size;
        *block = 0;  // 触发页分配
    }
}
```

**批量分配优化**

```c
// 一次分配多个块，减少函数调用次数
size_t mempool_alloc_batch(mempool *pool, void **ptrs, size_t count) {
    size_t actual = (count < pool->free_count) ? count : pool->free_count;
    for (size_t i = 0; i < actual; i++) {
        ptrs[i] = mempool_alloc(pool);
    }
    return actual;
}
```

## 注意事项

**容量预估**

池容量固定，需要根据场景预估：
- 估算过小：运行时耗尽，导致分配失败
- 估算过大：内存浪费

可考虑动态扩容方案，但会增加复杂度。

**生命周期管理**

```c
// 错误：池销毁后使用指针
mempool *pool = mempool_create(64, 100);
void *p = mempool_alloc(pool);
mempool_destroy(pool);
// p 现在是悬空指针！

// 正确：先归还所有块，确保池不再使用后再销毁
mempool_free(pool, p);
mempool_destroy(pool);
```

**线程安全**

默认实现非线程安全：
- 单线程场景：直接使用
- 多线程场景：加锁或使用 per-thread 池

**指针归属检查**

`mempool_free` 会检查指针是否属于该池：

```c
mempool *pool1 = mempool_create(64, 100);
mempool *pool2 = mempool_create(64, 100);

void *p = mempool_alloc(pool1);
mempool_free(pool2, p);  // 警告：指针不属于此池
```

**与标准分配混用**

```c
mempool *pool = mempool_create(64, 100);
void *p = mempool_alloc(pool);

free(p);  // 错误！池分配的内存不能用 free
mempool_free(pool, p);  // 正确
```

**可用性检查**

分配前检查返回值：

```c
void *p = mempool_alloc(pool);
if (!p) {
    // 池已空，降级处理
    p = malloc(size);  // 或返回错误
}
```

## 两种方案对比

| 特性 | 固定大小池 | Arena |
|------|-----------|-------|
| 块大小 | 固定 | 任意 |
| 单独释放 | 支持 | 不支持 |
| 分配速度 | O(1) | O(1) |
| 内存利用率 | 固定块可能浪费 | 紧凑无浪费 |
| 实现复杂度 | 中等 | 简单 |
| 适用场景 | 对象频繁创建销毁 | 批量分配统一释放 |
