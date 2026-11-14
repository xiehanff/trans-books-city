#include "test_common.h"

#include <stdint.h>

#include "memory_pool.h"

void test_memory_pool(void) {
    TEST_ASSERT(mempool_create(0, 1) == NULL);
    TEST_ASSERT(mempool_create(1, 0) == NULL);
    TEST_ASSERT(mempool_available(NULL) == 0);
    TEST_ASSERT(mempool_capacity(NULL) == 0);
    TEST_ASSERT(mempool_block_size(NULL) == 0);
    TEST_ASSERT(mempool_alloc(NULL) == NULL);
    TEST_ASSERT(mempool_free(NULL, NULL) == 0);
    mempool_destroy(NULL);

    mempool *pool = mempool_create(sizeof(int), 3);
    TEST_ASSERT(pool != NULL);
    TEST_ASSERT(mempool_capacity(pool) == 3);
    TEST_ASSERT(mempool_block_size(pool) >= sizeof(int));
    TEST_ASSERT(mempool_available(pool) == 3);

    void *first = mempool_alloc(pool);
    void *second = mempool_alloc(pool);
    void *third = mempool_alloc(pool);
    TEST_ASSERT(first != NULL && second != NULL && third != NULL);
    TEST_ASSERT(mempool_alloc(pool) == NULL);
    TEST_ASSERT(mempool_available(pool) == 0);
    *(int *)first = 11;
    *(int *)second = 22;
    TEST_ASSERT(*(int *)first == 11 && *(int *)second == 22);

    TEST_ASSERT(mempool_free(pool, second));
    TEST_ASSERT(mempool_available(pool) == 1);
    TEST_ASSERT(!mempool_free(pool, second));
    TEST_ASSERT(!mempool_free(pool, (unsigned char *)first + 1));
    int outside = 0;
    TEST_ASSERT(!mempool_free(pool, &outside));
    void *reused = mempool_alloc(pool);
    TEST_ASSERT(reused != NULL && mempool_available(pool) == 0);
    *(int *)reused = 33;
    TEST_ASSERT(*(int *)reused == 33);
    TEST_ASSERT(mempool_free(pool, first));
    TEST_ASSERT(mempool_free(pool, third));
    TEST_ASSERT(mempool_free(pool, reused));
    TEST_ASSERT(mempool_available(pool) == mempool_capacity(pool));
    mempool_destroy(pool);

    arena *arena_pool = arena_create(64);
    TEST_ASSERT(arena_pool != NULL);
    TEST_ASSERT(arena_capacity(arena_pool) == 64 && arena_used(arena_pool) == 0);
    TEST_ASSERT(arena_alloc(arena_pool, 0) == NULL);
    TEST_ASSERT(arena_alloc(arena_pool, SIZE_MAX) == NULL);
    unsigned char *bytes = arena_alloc(arena_pool, 5);
    TEST_ASSERT(bytes != NULL && arena_used(arena_pool) >= 5);
    void *zeroed = arena_calloc(arena_pool, 8);
    TEST_ASSERT(zeroed != NULL);
    for (size_t i = 0; i < 8; i++) {
        TEST_ASSERT(((unsigned char *)zeroed)[i] == 0);
    }
    TEST_ASSERT(arena_alloc(arena_pool, 64) == NULL);
    arena_reset(arena_pool);
    TEST_ASSERT(arena_used(arena_pool) == 0);
    TEST_ASSERT(arena_alloc(arena_pool, 64) != NULL);
    arena_destroy(arena_pool);
    TEST_ASSERT(arena_create(0) == NULL);
    TEST_ASSERT(arena_alloc(NULL, 1) == NULL);
    TEST_ASSERT(arena_calloc(NULL, 1) == NULL);
    arena_reset(NULL);
    arena_destroy(NULL);
    TEST_ASSERT(arena_used(NULL) == 0 && arena_capacity(NULL) == 0);
}
