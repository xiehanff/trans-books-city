#ifndef C_DOC_TEST_MEMORY_POOL_H
#define C_DOC_TEST_MEMORY_POOL_H

#include <stddef.h>

typedef struct mempool mempool;

typedef struct arena arena;

mempool *mempool_create(size_t block_size, size_t count);
void *mempool_alloc(mempool *pool);
int mempool_free(mempool *pool, void *pointer);
void mempool_destroy(mempool *pool);
size_t mempool_available(const mempool *pool);
size_t mempool_capacity(const mempool *pool);
size_t mempool_block_size(const mempool *pool);

arena *arena_create(size_t capacity);
void *arena_alloc(arena *arena_pool, size_t size);
void *arena_calloc(arena *arena_pool, size_t size);
void arena_reset(arena *arena_pool);
void arena_destroy(arena *arena_pool);
size_t arena_used(const arena *arena_pool);
size_t arena_capacity(const arena *arena_pool);

#endif
