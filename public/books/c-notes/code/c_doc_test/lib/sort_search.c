#include "sort_search.h"

#include <limits.h>

static void swap_int(int *left, int *right) {
    int temporary = *left;
    *left = *right;
    *right = temporary;
}

void bubble_sort(int *array, size_t count) {
    if (array == NULL || count < 2) {
        return;
    }

    for (size_t i = 0; i + 1 < count; i++) {
        for (size_t j = 0; j + 1 < count - i; j++) {
            if (array[j] > array[j + 1]) {
                swap_int(&array[j], &array[j + 1]);
            }
        }
    }
}

void bubble_sort_optimized(int *array, size_t count) {
    if (array == NULL || count < 2) {
        return;
    }

    for (size_t i = 0; i + 1 < count; i++) {
        int swapped = 0;
        for (size_t j = 0; j + 1 < count - i; j++) {
            if (array[j] > array[j + 1]) {
                swap_int(&array[j], &array[j + 1]);
                swapped = 1;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

void selection_sort(int *array, size_t count) {
    if (array == NULL || count < 2) {
        return;
    }

    for (size_t i = 0; i + 1 < count; i++) {
        size_t minimum = i;
        for (size_t j = i + 1; j < count; j++) {
            if (array[j] < array[minimum]) {
                minimum = j;
            }
        }
        if (minimum != i) {
            swap_int(&array[i], &array[minimum]);
        }
    }
}

void insertion_sort(int *array, size_t count) {
    if (array == NULL || count < 2) {
        return;
    }

    for (size_t i = 1; i < count; i++) {
        int key = array[i];
        size_t j = i;
        while (j > 0 && array[j - 1] > key) {
            array[j] = array[j - 1];
            j--;
        }
        array[j] = key;
    }
}

static size_t partition(int *array, size_t low, size_t high) {
    int pivot = array[high - 1];
    size_t store = low;

    for (size_t i = low; i + 1 < high; i++) {
        if (array[i] <= pivot) {
            swap_int(&array[store], &array[i]);
            store++;
        }
    }
    swap_int(&array[store], &array[high - 1]);
    return store;
}

static void quick_sort_range(int *array, size_t low, size_t high) {
    if (high - low < 2) {
        return;
    }

    size_t pivot = partition(array, low, high);
    quick_sort_range(array, low, pivot);
    if (pivot + 1 < high) {
        quick_sort_range(array, pivot + 1, high);
    }
}

void quick_sort(int *array, size_t count) {
    if (array == NULL || count < 2) {
        return;
    }
    quick_sort_range(array, 0, count);
}

static void merge_ranges(int *array, int *temporary,
                         size_t left, size_t middle, size_t right) {
    size_t first = left;
    size_t second = middle;
    size_t output = left;

    while (first < middle && second < right) {
        if (array[first] <= array[second]) {
            temporary[output++] = array[first++];
        } else {
            temporary[output++] = array[second++];
        }
    }
    while (first < middle) {
        temporary[output++] = array[first++];
    }
    while (second < right) {
        temporary[output++] = array[second++];
    }
    for (size_t i = left; i < right; i++) {
        array[i] = temporary[i];
    }
}

static void merge_sort_range(int *array, int *temporary,
                             size_t left, size_t right) {
    if (right - left < 2) {
        return;
    }

    size_t middle = left + (right - left) / 2;
    merge_sort_range(array, temporary, left, middle);
    merge_sort_range(array, temporary, middle, right);
    merge_ranges(array, temporary, left, middle, right);
}

void merge_sort(int *array, int *temporary, size_t count) {
    if (array == NULL || temporary == NULL || count < 2) {
        return;
    }
    merge_sort_range(array, temporary, 0, count);
}

int linear_search(const int *array, size_t count, int target) {
    if (array == NULL || count > (size_t)INT_MAX) {
        return -1;
    }

    for (size_t i = 0; i < count; i++) {
        if (array[i] == target) {
            return (int)i;
        }
    }
    return -1;
}

int binary_search(const int *array, size_t count, int target) {
    if (array == NULL || count > (size_t)INT_MAX) {
        return -1;
    }

    size_t left = 0;
    size_t right = count;
    while (left < right) {
        size_t middle = left + (right - left) / 2;
        if (array[middle] == target) {
            return (int)middle;
        }
        if (array[middle] < target) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }
    return -1;
}

int find_first(const int *array, size_t count, int target) {
    if (array == NULL || count > (size_t)INT_MAX) {
        return -1;
    }

    size_t left = 0;
    size_t right = count;
    size_t result = count;
    while (left < right) {
        size_t middle = left + (right - left) / 2;
        if (array[middle] == target) {
            result = middle;
            right = middle;
        } else if (array[middle] < target) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }
    return result == count ? -1 : (int)result;
}

int find_first_greater(const int *array, size_t count, int target) {
    if (array == NULL || count > (size_t)INT_MAX) {
        return -1;
    }

    size_t left = 0;
    size_t right = count;
    while (left < right) {
        size_t middle = left + (right - left) / 2;
        if (array[middle] <= target) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }
    return left == count ? -1 : (int)left;
}
