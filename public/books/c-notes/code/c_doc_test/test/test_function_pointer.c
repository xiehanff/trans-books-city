#include "test_common.h"

#include <limits.h>
#include <stdlib.h>

#include "function_pointer.h"

static void add_offset(int *value, void *context) {
    const int offset = *(const int *)context;
    *value += offset;
}

void test_function_pointer(void) {
    int result = 0;

    TEST_ASSERT(apply_operation(add, 2, 3, &result));
    TEST_ASSERT(result == 5);
    TEST_ASSERT(apply_operation(subtract, 2, 3, &result));
    TEST_ASSERT(result == -1);
    TEST_ASSERT(apply_operation(multiply, 2, 3, &result));
    TEST_ASSERT(result == 6);
    TEST_ASSERT(add(INT_MAX, 1) == INT_MAX);
    TEST_ASSERT(subtract(INT_MIN, 1) == INT_MIN);
    TEST_ASSERT(multiply(INT_MAX, 2) == INT_MAX);
    TEST_ASSERT(multiply(INT_MIN, 2) == INT_MIN);
    TEST_ASSERT(multiply(INT_MIN, -1) == INT_MAX);
    TEST_ASSERT(!apply_operation(NULL, 2, 3, &result));
    TEST_ASSERT(!apply_operation(add, 2, 3, NULL));

    BinaryOperation operations[] = {add, subtract, multiply};
    TEST_ASSERT(operations[0](8, 2) == 10);
    TEST_ASSERT(operations[1](8, 2) == 6);
    TEST_ASSERT(operations[2](8, 2) == 16);

    int values[] = {1, 2, 3};
    const int offset = 10;
    visit_ints(values, sizeof values / sizeof values[0], add_offset,
               (void *)&offset);
    TEST_ASSERT(values[0] == 11 && values[1] == 12 && values[2] == 13);
    visit_ints(NULL, 0, add_offset, (void *)&offset);
    visit_ints(NULL, 1, add_offset, (void *)&offset);

    int sortable[] = {4, -2, 9, 0};
    qsort(sortable, sizeof sortable / sizeof sortable[0], sizeof sortable[0],
          compare_int_ascending);
    TEST_ASSERT(sortable[0] == -2);
    TEST_ASSERT(sortable[1] == 0);
    TEST_ASSERT(sortable[2] == 4);
    TEST_ASSERT(sortable[3] == 9);

    /* int *(*factory)(int) 是“指向返回 int* 的函数的指针”。 */
    int *(*factory)(int) = create_int;
    int *created = factory(42);
    TEST_ASSERT(created != NULL && *created == 42);
    free(created);
}

