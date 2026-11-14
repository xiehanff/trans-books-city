#include "basic_examples.h"
#include "test_common.h"

#include <limits.h>
#include <string.h>

void test_basic_examples(void)
{
    int rhs_evaluated = -1;
    int value = 10;
    int values[] = {4, 7, 1, 7};
    BasicPoint point = {.x = 2, .y = -1};
    char text[] = "hello";

    TEST_ASSERT(strcmp(basic_hello_message(), "Hello, World!") == 0);
    TEST_ASSERT(basic_constant_value() == 62);
    TEST_ASSERT(sizeof(char) == 1U);
    TEST_ASSERT(sizeof(short) <= sizeof(int));
    TEST_ASSERT(sizeof(int) <= sizeof(long));
    TEST_ASSERT(sizeof(long) <= sizeof(long long));
    TEST_ASSERT(sizeof(float) > 0U);
    TEST_ASSERT(sizeof(double) > 0U);
    TEST_ASSERT(sizeof(void *) > 0U);
    TEST_ASSERT(sizeof values / sizeof values[0] == 4U);

    TEST_ASSERT(5 / 2 == 2);
    TEST_ASSERT(basic_divide_as_double(5, 2) == 2.5);
    TEST_ASSERT((int)3.75 == 3);
    TEST_ASSERT('A' + 1 == 'B');

    TEST_ASSERT(basic_short_circuit_and(0, 0, &rhs_evaluated) == 0);
    TEST_ASSERT(rhs_evaluated == 0);
    TEST_ASSERT(basic_short_circuit_and(1, 0, &rhs_evaluated) == 0);
    TEST_ASSERT(rhs_evaluated == 1);
    TEST_ASSERT(basic_short_circuit_or(1, 0, &rhs_evaluated) == 1);
    TEST_ASSERT(rhs_evaluated == 0);
    TEST_ASSERT(basic_short_circuit_or(0, 1, &rhs_evaluated) == 1);
    TEST_ASSERT(rhs_evaluated == 1);
    TEST_ASSERT(basic_conditional_max(3, 8) == 8);

    TEST_ASSERT(basic_increment_by_value(value) == 11);
    TEST_ASSERT(value == 10);
    basic_increment_by_pointer(&value);
    TEST_ASSERT(value == 11);
    TEST_ASSERT(basic_factorial(0U) == 1ULL);
    TEST_ASSERT(basic_factorial(5U) == 120ULL);
    TEST_ASSERT(basic_factorial(21U) == 0ULL);

    TEST_ASSERT(basic_array_sum(values, sizeof values / sizeof values[0]) == 19);
    TEST_ASSERT(basic_find_index(values, sizeof values / sizeof values[0], 7) == 1);
    TEST_ASSERT(basic_find_index(values, sizeof values / sizeof values[0], 9) == -1);
    basic_translate_point(&point, 3, 4);
    TEST_ASSERT(point.x == 5 && point.y == 3);
    point.x = INT_MAX;
    point.y = INT_MIN;
    basic_translate_point(&point, 1, -1);
    TEST_ASSERT(point.x == INT_MAX && point.y == INT_MIN);
    TEST_ASSERT(basic_array_sum((const int[]){INT_MAX, 1}, 2) > 0);
    TEST_ASSERT(basic_string_length(text) == 5U);
    TEST_ASSERT(basic_string_length(NULL) == 0U);
}
