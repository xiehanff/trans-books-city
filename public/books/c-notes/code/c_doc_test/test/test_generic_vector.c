#include "test_common.h"

#include <stdint.h>

#include "generic_vector.h"

void test_generic_vector(void) {
    TEST_ASSERT(vec_create(0, 1) == NULL);
    TEST_ASSERT(vec_reserve(NULL, 1) == 0);
    TEST_ASSERT(vec_push(NULL, &(int){1}) == 0);
    TEST_ASSERT(vec_get(NULL, 0) == NULL);
    TEST_ASSERT(vec_shrink_to_fit(NULL) == 0);
    vec_free(NULL);

    vector *numbers = vec_create(sizeof(int), 0);
    TEST_ASSERT(numbers != NULL && numbers->size == 0 && numbers->capacity == 0);
    int value = 10;
    TEST_ASSERT(vec_push(numbers, &value));
    value = 20;
    TEST_ASSERT(vec_push(numbers, &value));
    value = 30;
    TEST_ASSERT(vec_push(numbers, &value));
    TEST_ASSERT(numbers->size == 3 && numbers->capacity >= 3);
    TEST_ASSERT(*(const int *)vec_get_const(numbers, 1) == 20);
    TEST_ASSERT(vec_get(numbers, 3) == NULL);
    TEST_ASSERT(vec_get_const(NULL, 0) == NULL);
    TEST_ASSERT(vec_set(numbers, 1, &(int){200}));
    TEST_ASSERT(*(int *)vec_get(numbers, 1) == 200);
    TEST_ASSERT(!vec_set(numbers, 3, &value));
    TEST_ASSERT(!vec_set(numbers, 0, NULL));

    value = 15;
    TEST_ASSERT(vec_insert(numbers, 1, &value));
    TEST_ASSERT(numbers->size == 4);
    TEST_ASSERT(*(int *)vec_get(numbers, 0) == 10);
    TEST_ASSERT(*(int *)vec_get(numbers, 1) == 15);
    TEST_ASSERT(*(int *)vec_get(numbers, 2) == 200);
    TEST_ASSERT(*(int *)vec_get(numbers, 3) == 30);
    value = 40;
    TEST_ASSERT(vec_insert(numbers, numbers->size, &value));
    TEST_ASSERT(vec_erase(numbers, 1));
    TEST_ASSERT(*(int *)vec_get(numbers, 1) == 200);
    TEST_ASSERT(!vec_erase(numbers, numbers->size));

    int batch[] = {50, 60, 70};
    TEST_ASSERT(vec_push_n(numbers, batch, 3));
    TEST_ASSERT(numbers->size == 7);
    TEST_ASSERT(*(int *)vec_get(numbers, 6) == 70);
    TEST_ASSERT(vec_push_n(numbers, NULL, 0));
    TEST_ASSERT(!vec_push_n(numbers, NULL, 1));

    int popped = 0;
    TEST_ASSERT(vec_pop(numbers, &popped) && popped == 70);
    TEST_ASSERT(vec_pop(numbers, NULL));
    while (numbers->size > 0) {
        TEST_ASSERT(vec_pop(numbers, NULL));
    }
    TEST_ASSERT(!vec_pop(numbers, &popped));
    TEST_ASSERT(vec_shrink_to_fit(numbers));
    TEST_ASSERT(numbers->capacity == 0 && numbers->data == NULL);

    value = 1;
    TEST_ASSERT(vec_push(numbers, &value));
    TEST_ASSERT(vec_reserve(numbers, 32));
    TEST_ASSERT(numbers->capacity >= 32);
    TEST_ASSERT(vec_shrink_to_fit(numbers));
    TEST_ASSERT(numbers->capacity == numbers->size);
    vec_clear(numbers);
    TEST_ASSERT(numbers->size == 0 && numbers->capacity > 0);

    /* 追加和预留都必须在 size_t 乘法溢出前失败。 */
    const size_t saved_capacity = numbers->capacity;
    TEST_ASSERT(!vec_reserve(numbers, SIZE_MAX));
    TEST_ASSERT(numbers->capacity == saved_capacity);
    const size_t saved_size = numbers->size;
    numbers->size = SIZE_MAX;
    TEST_ASSERT(!vec_push(numbers, &value));
    TEST_ASSERT(!vec_push_n(numbers, &value, 1));
    numbers->size = saved_size;

    /* value 指向容器自身时，插入仍应先复制再移动。 */
    value = 99;
    TEST_ASSERT(vec_push(numbers, &value));
    TEST_ASSERT(vec_insert(numbers, 0, vec_get(numbers, 0)));
    TEST_ASSERT(*(int *)vec_get(numbers, 0) == *(int *)vec_get(numbers, 1));

    vec_free(numbers);
}
