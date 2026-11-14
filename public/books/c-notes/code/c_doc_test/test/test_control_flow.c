#include "control_flow.h"
#include "test_common.h"

void test_control_flow(void)
{
    const int values[] = {-3, 0, 2, 5, 5};
    const size_t count = sizeof values / sizeof values[0];

    TEST_ASSERT(control_grade_level(95) == 3);
    TEST_ASSERT(control_grade_level(85) == 2);
    TEST_ASSERT(control_grade_level(70) == 1);
    TEST_ASSERT(control_grade_level(40) == 0);

    TEST_ASSERT(control_day_kind(1) == 1);
    TEST_ASSERT(control_day_kind(5) == 1);
    TEST_ASSERT(control_day_kind(6) == 2);
    TEST_ASSERT(control_day_kind(7) == 2);
    TEST_ASSERT(control_day_kind(0) == 0);

    TEST_ASSERT(control_sum_while(5) == 10);
    TEST_ASSERT(control_sum_while(-1) == 0);
    TEST_ASSERT(control_sum_do_while(0, 5) == 10);
    TEST_ASSERT(control_sum_do_while(5, 0) == 0);
    TEST_ASSERT(control_sum_for(5) == 10);
    TEST_ASSERT(control_sum_for(-1) == 0);

    TEST_ASSERT(control_count_odd(values, count) == 3U);
    TEST_ASSERT(control_find_first(values, count, 5) == 3);
    TEST_ASSERT(control_find_first(values, count, 9) == -1);
}
