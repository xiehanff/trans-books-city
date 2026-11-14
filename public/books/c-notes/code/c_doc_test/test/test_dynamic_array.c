#include "test_common.h"

#include "dynamic_array.h"

void test_dynamic_array(void) {
    IntArray numbers;
    int_array_init(&numbers);
    TEST_ASSERT(numbers.data == NULL);
    TEST_ASSERT(numbers.size == 0 && numbers.capacity == 0);

    for (int i = 0; i < 10; i++) {
        TEST_ASSERT(int_array_push(&numbers, i * i));
    }
    TEST_ASSERT(numbers.size == 10);
    TEST_ASSERT(numbers.capacity >= numbers.size);

    int value = 0;
    TEST_ASSERT(int_array_get(&numbers, 0, &value) && value == 0);
    TEST_ASSERT(int_array_get(&numbers, 9, &value) && value == 81);
    TEST_ASSERT(!int_array_get(&numbers, 10, &value));
    TEST_ASSERT(!int_array_get(&numbers, 0, NULL));

    TEST_ASSERT(int_array_reserve(&numbers, 32));
    TEST_ASSERT(numbers.capacity >= 32);
    for (size_t i = 0; i < numbers.size; i++) {
        TEST_ASSERT(int_array_get(&numbers, i, &value));
        TEST_ASSERT(value == (int)(i * i));
    }

    TEST_ASSERT(int_array_pop(&numbers, &value) && value == 81);
    TEST_ASSERT(numbers.size == 9);
    TEST_ASSERT(int_array_pop(&numbers, NULL));
    TEST_ASSERT(numbers.size == 8);

    int_array_free(&numbers);
    TEST_ASSERT(numbers.data == NULL);
    TEST_ASSERT(numbers.size == 0 && numbers.capacity == 0);
    int_array_free(&numbers);

    TEST_ASSERT(!int_array_push(NULL, 1));
    TEST_ASSERT(!int_array_reserve(NULL, 1));
}

