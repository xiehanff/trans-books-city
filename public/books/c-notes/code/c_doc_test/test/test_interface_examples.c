#include "test_common.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "interface_examples.h"

#define C_DOC_GENERATED_INDEX 42
static int C_DOC_TOKEN_PASTE(c_doc_generated_value_, C_DOC_GENERATED_INDEX) = 7;

void test_interface_examples(void) {
    int value = 10;
    int other = 20;
    int output = 0;

    TEST_ASSERT(interface_set_int(&value, 11));
    TEST_ASSERT(value == 11);
    TEST_ASSERT(!interface_set_int(NULL, 11));

    const int read_only = 37;
    TEST_ASSERT(interface_read_int(&read_only, &output));
    TEST_ASSERT(output == 37);
    TEST_ASSERT(!interface_read_int(NULL, &output));
    TEST_ASSERT(!interface_read_int(&read_only, NULL));

    int *pointer = NULL;
    TEST_ASSERT(interface_set_int_pointer(&pointer, &value));
    TEST_ASSERT(pointer == &value);
    TEST_ASSERT(interface_read_pointer_chain(&pointer, &output));
    TEST_ASSERT(output == 11);
    TEST_ASSERT(!interface_set_int_pointer(NULL, &value));
    TEST_ASSERT(!interface_read_pointer_chain(NULL, &output));
    pointer = NULL;
    TEST_ASSERT(!interface_read_pointer_chain(&pointer, &output));
    TEST_ASSERT(!interface_read_pointer_chain(&pointer, NULL));

    TEST_ASSERT(interface_swap_int(&value, &other));
    TEST_ASSERT(value == 20 && other == 11);
    TEST_ASSERT(!interface_swap_int(NULL, &other));
    TEST_ASSERT(!interface_swap_int(&value, NULL));

    int quotient = 0;
    int remainder = 0;
    TEST_ASSERT(interface_divide(17, 5, &quotient, &remainder));
    TEST_ASSERT(quotient == 3 && remainder == 2);
    TEST_ASSERT(!interface_divide(17, 0, &quotient, &remainder));
    TEST_ASSERT(!interface_divide(INT_MIN, -1, &quotient, &remainder));
    TEST_ASSERT(!interface_divide(17, 5, NULL, &remainder));
    TEST_ASSERT(!interface_divide(17, 5, &quotient, NULL));
    TEST_ASSERT(!interface_divide(17, 5, &quotient, &quotient));

    const int values[] = {1, 2, 3};
    int sum = -1;
    TEST_ASSERT(interface_sum_ints(values, C_DOC_ARRAY_SIZE(values), &sum));
    TEST_ASSERT(sum == 6);
    TEST_ASSERT(interface_sum_ints(NULL, 0, &sum) && sum == 0);
    TEST_ASSERT(!interface_sum_ints(NULL, 1, &sum));
    TEST_ASSERT(!interface_sum_ints(values, C_DOC_ARRAY_SIZE(values), NULL));
    const int overflowing[] = {INT_MAX, 1};
    TEST_ASSERT(!interface_sum_ints(overflowing, 2, &sum));

    TEST_ASSERT(interface_compare_int(NULL, NULL) == 0);
    TEST_ASSERT(interface_compare_int(NULL, &value) < 0);
    TEST_ASSERT(interface_compare_int(&value, NULL) > 0);
    const int low = INT_MIN;
    const int high = INT_MAX;
    TEST_ASSERT(interface_compare_int(&low, &high) < 0);
    TEST_ASSERT(interface_compare_int(&high, &low) > 0);

    int sortable[] = {INT_MAX, 0, INT_MIN, 7, -7};
    TEST_ASSERT(interface_sort_ints(sortable, C_DOC_ARRAY_SIZE(sortable)));
    TEST_ASSERT(sortable[0] == INT_MIN && sortable[1] == -7);
    TEST_ASSERT(sortable[2] == 0 && sortable[3] == 7);
    TEST_ASSERT(sortable[4] == INT_MAX);
    TEST_ASSERT(interface_sort_ints(NULL, 0));
    TEST_ASSERT(!interface_sort_ints(NULL, 1));

    size_t index = 0;
    TEST_ASSERT(interface_find_int(sortable, C_DOC_ARRAY_SIZE(sortable), 7,
                                   &index));
    TEST_ASSERT(index < C_DOC_ARRAY_SIZE(sortable) && sortable[index] == 7);
    TEST_ASSERT(!interface_find_int(sortable, C_DOC_ARRAY_SIZE(sortable), 8,
                                    &index));
    TEST_ASSERT(index == SIZE_MAX);
    TEST_ASSERT(!interface_find_int(NULL, 0, 8, &index));
    TEST_ASSERT(index == SIZE_MAX);
    TEST_ASSERT(!interface_find_int(sortable, C_DOC_ARRAY_SIZE(sortable), 7,
                                    NULL));

    double average = -1.0;
    TEST_ASSERT(interface_average_ints(4, &average, 10, 20, 30, 40));
    TEST_ASSERT(average == 25.0);
    TEST_ASSERT(interface_average_ints(0, &average) && average == 0.0);
    TEST_ASSERT(!interface_average_ints(1, NULL, 10));

    TEST_ASSERT(C_DOC_SQUARE(1 + 2) == 9);
    TEST_ASSERT(C_DOC_STRINGIFY(C_DOC_GENERATED_INDEX)[0] == '4');
    TEST_ASSERT(strcmp(C_DOC_STRINGIFY(C_DOC_GENERATED_INDEX), "42") == 0);
    TEST_ASSERT(C_DOC_TOKEN_PASTE(c_doc_generated_value_,
                                 C_DOC_GENERATED_INDEX) == 7);

    int left = 1;
    int right = 2;
    if (left < right) {
        C_DOC_SWAP_INT(left, right);
    } else {
        C_DOC_SWAP_INT(right, left);
    }
    TEST_ASSERT(left == 2 && right == 1);
}
