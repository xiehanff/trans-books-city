#include "test_common.h"

#include <limits.h>

#include "sort_search.h"

static void assert_sorted(const int *array, size_t count) {
    for (size_t i = 1; i < count; i++) {
        TEST_ASSERT(array[i - 1] <= array[i]);
    }
}

void test_sort_search(void) {
    /* 第12章：冒泡、选择、插入、快速和归并排序。 */
    const int source[] = {5, -2, 8, 5, 0, -2, 1};
    const size_t count = sizeof source / sizeof source[0];
    int values[sizeof source / sizeof source[0]];
    int temporary[sizeof source / sizeof source[0]];

    for (int algorithm = 0; algorithm < 6; algorithm++) {
        for (size_t i = 0; i < count; i++) {
            values[i] = source[i];
        }
        switch (algorithm) {
        case 0:
            bubble_sort(values, count);
            break;
        case 1:
            bubble_sort_optimized(values, count);
            break;
        case 2:
            selection_sort(values, count);
            break;
        case 3:
            insertion_sort(values, count);
            break;
        case 4:
            quick_sort(values, count);
            break;
        default:
            merge_sort(values, temporary, count);
            break;
        }
        assert_sorted(values, count);
        TEST_ASSERT(values[0] == -2 && values[count - 1] == 8);
    }

    bubble_sort(NULL, 0);
    quick_sort(NULL, 0);
    merge_sort(NULL, NULL, 0);

    /* 第12章：无序数组使用顺序查找，有序数组使用二分查找。 */
    const int unsorted[] = {7, -3, 7, 4};
    TEST_ASSERT(linear_search(unsorted, 4, -3) == 1);
    TEST_ASSERT(linear_search(unsorted, 4, 99) == -1);
    TEST_ASSERT(linear_search(NULL, 0, 1) == -1);

    const int sorted[] = {-3, 1, 4, 4, 4, 9, 12};
    int found = binary_search(sorted, 7, 4);
    TEST_ASSERT(found >= 2 && found <= 4);
    TEST_ASSERT(binary_search(sorted, 7, 5) == -1);
    TEST_ASSERT(find_first(sorted, 7, 4) == 2);
    TEST_ASSERT(find_first(sorted, 7, -3) == 0);
    TEST_ASSERT(find_first(sorted, 7, 5) == -1);
    TEST_ASSERT(find_first_greater(sorted, 7, 4) == 5);
    TEST_ASSERT(find_first_greater(sorted, 7, 12) == -1);
    TEST_ASSERT(find_first_greater(sorted, 0, 12) == -1);

    if ((size_t)-1 > (size_t)INT_MAX) {
        const int one = 1;
        const size_t too_many = (size_t)INT_MAX + 1U;
        TEST_ASSERT(linear_search(&one, too_many, 1) == -1);
        TEST_ASSERT(binary_search(&one, too_many, 1) == -1);
        TEST_ASSERT(find_first(&one, too_many, 1) == -1);
        TEST_ASSERT(find_first_greater(&one, too_many, 1) == -1);
    }
}
