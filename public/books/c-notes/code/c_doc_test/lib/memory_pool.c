#include "memory_pool.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define POOL_ALIGNMENT _Alignof(max_align_t)

struct mempool {
    size_t block_size;
    size_t capacity;
    size_t free_count;
    unsigned char *memory;
    size_t *free_indices;
    unsigned char *in_use;
};

struct arena {
    unsigned char *buffer;
    size_t capacity;
    size_t offset;
};

static int align_size(size_t size, size_t alignment, size_t *result) {
    const size_t remainder = size % alignment;
    if (remainder == 0) {
        *result = size;
        return 1;
    }
    const size_t adjustment = alignment - remainder;
    if (size > SIZE_MAX - adjustment) {
        return 0;
    }
    *result = size + adjustment;
    return 1;
}

mempool *mempool_create(size_t block_size, size_t count) {
    if (block_size == 0 || count == 0) {
        return NULL;
    }

    size_t aligned_block_size = 0;
    if (!align_size(block_size, POOL_ALIGNMENT, &aligned_block_size) ||
        aligned_block_size > SIZE_MAX / count ||
        count > SIZE_MAX / sizeof(size_t)) {
        return NULL;
    }

    mempool *pool = malloc(sizeof *pool);
    if (pool == NULL) {
        return NULL;
    }
    pool->block_size = aligned_block_size;
    pool->capacity = count;
    pool->free_count = count;
    pool->memory = malloc(aligned_block_size * count);
    pool->free_indices = malloc(count * sizeof *pool->free_indices);
    pool->in_use = calloc(count, sizeof *pool->in_use);
    if (pool->memory == NULL || pool->free_indices == NULL || pool->in_use == NULL) {
        free(pool->in_use);
        free(pool->free_indices);
        free(pool->memory);
        free(pool);
        return NULL;
    }
    for (size_t i = 0; i < count; i++) {
        pool->free_indices[i] = count - i - 1;
    }
    return pool;
}

void *mempool_alloc(mempool *pool) {
    if (pool == NULL || pool->free_count == 0) {
        return NULL;
    }
    const size_t index = pool->free_indices[pool->free_count - 1];
    pool->free_count--;
    pool->in_use[index] = 1;
    return pool->memory + index * pool->block_size;
}

int mempool_free(mempool *pool, void *pointer) {
    if (pool == NULL || pointer == NULL) {
        return 0;
    }

    /* 只用相等比较定位块，避免把任意指针转换为整数后做地址运算。 */
    size_t index = pool->capacity;
    for (size_t i = 0; i < pool->capacity; i++) {
        if (pointer == pool->memory + i * pool->block_size) {
            index = i;
            break;
        }
    }
    if (index == pool->capacity || pool->in_use[index] == 0 ||
        pool->free_count >= pool->capacity) {
        return 0;
    }

    pool->in_use[index] = 0;
    pool->free_indices[pool->free_count] = index;
    pool->free_count++;
    return 1;
}

void mempool_destroy(mempool *pool) {
    if (pool == NULL) {
        return;
    }
    free(pool->in_use);
    free(pool->free_indices);
    free(pool->memory);
    free(pool);
}

size_t mempool_available(const mempool *pool) {
    return pool == NULL ? 0 : pool->free_count;
}

size_t mempool_capacity(const mempool *pool) {
    return pool == NULL ? 0 : pool->capacity;
}

size_t mempool_block_size(const mempool *pool) {
    return pool == NULL ? 0 : pool->block_size;
}

arena *arena_create(size_t capacity) {
    if (capacity == 0) {
        return NULL;
    }
    arena *arena_pool = malloc(sizeof *arena_pool);
    if (arena_pool == NULL) {
        return NULL;
    }
    arena_pool->buffer = malloc(capacity);
    if (arena_pool->buffer == NULL) {
        free(arena_pool);
        return NULL;
    }
    arena_pool->capacity = capacity;
    arena_pool->offset = 0;
    return arena_pool;
}

void *arena_alloc(arena *arena_pool, size_t size) {
    if (arena_pool == NULL || size == 0) {
        return NULL;
    }

    size_t aligned_offset = 0;
    size_t aligned_size = 0;
    if (!align_size(arena_pool->offset, POOL_ALIGNMENT, &aligned_offset) ||
        !align_size(size, POOL_ALIGNMENT, &aligned_size) ||
        aligned_offset > arena_pool->capacity ||
        aligned_size > arena_pool->capacity - aligned_offset) {
        return NULL;
    }

    void *result = arena_pool->buffer + aligned_offset;
    arena_pool->offset = aligned_offset + aligned_size;
    return result;
}

void *arena_calloc(arena *arena_pool, size_t size) {
    void *result = arena_alloc(arena_pool, size);
    if (result != NULL) {
        memset(result, 0, size);
    }
    return result;
}

void arena_reset(arena *arena_pool) {
    if (arena_pool != NULL) {
        arena_pool->offset = 0;
    }
}

void arena_destroy(arena *arena_pool) {
    if (arena_pool == NULL) {
        return;
    }
    free(arena_pool->buffer);
    free(arena_pool);
}

size_t arena_used(const arena *arena_pool) {
    return arena_pool == NULL ? 0 : arena_pool->offset;
}

size_t arena_capacity(const arena *arena_pool) {
    return arena_pool == NULL ? 0 : arena_pool->capacity;
}
