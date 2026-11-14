#ifndef C_DOC_TEST_SORT_SEARCH_H
#define C_DOC_TEST_SORT_SEARCH_H

#include <stddef.h>

void bubble_sort(int *array, size_t count);
void bubble_sort_optimized(int *array, size_t count);
void selection_sort(int *array, size_t count);
void insertion_sort(int *array, size_t count);
void quick_sort(int *array, size_t count);
void merge_sort(int *array, int *temporary, size_t count);

int linear_search(const int *array, size_t count, int target);
int binary_search(const int *array, size_t count, int target);
int find_first(const int *array, size_t count, int target);
int find_first_greater(const int *array, size_t count, int target);

#endif
