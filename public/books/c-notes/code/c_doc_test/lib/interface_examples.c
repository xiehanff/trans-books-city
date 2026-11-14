#include "interface_examples.h"

#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>

int interface_set_int(int *value, int replacement) {
    if (value == NULL) {
        return 0;
    }

    *value = replacement;
    return 1;
}

int interface_read_int(const int *value, int *output) {
    if (value == NULL || output == NULL) {
        return 0;
    }

    *output = *value;
    return 1;
}

int interface_set_int_pointer(int **slot, int *value) {
    if (slot == NULL) {
        return 0;
    }

    *slot = value;
    return 1;
}

int interface_read_pointer_chain(int *const *slot, int *output) {
    if (slot == NULL || *slot == NULL || output == NULL) {
        return 0;
    }

    *output = **slot;
    return 1;
}

int interface_swap_int(int *left, int *right) {
    if (left == NULL || right == NULL) {
        return 0;
    }

    int temporary = *left;
    *left = *right;
    *right = temporary;
    return 1;
}

int interface_divide(int dividend, int divisor, int *quotient, int *remainder) {
    if (divisor == 0 || quotient == NULL || remainder == NULL ||
        quotient == remainder || (dividend == INT_MIN && divisor == -1)) {
        return 0;
    }

    const int calculated_quotient = dividend / divisor;
    const int calculated_remainder = dividend % divisor;
    *quotient = calculated_quotient;
    *remainder = calculated_remainder;
    return 1;
}

int interface_sum_ints(const int *values, size_t count, int *output) {
    if (output == NULL || (values == NULL && count > 0)) {
        return 0;
    }

    int total = 0;
    for (size_t i = 0; i < count; i++) {
        const int value = values[i];
        if ((value > 0 && total > INT_MAX - value) ||
            (value < 0 && total < INT_MIN - value)) {
            return 0;
        }
        total += value;
    }

    *output = total;
    return 1;
}

int interface_compare_int(const void *left, const void *right) {
    if (left == right) {
        return 0;
    }
    if (left == NULL) {
        return -1;
    }
    if (right == NULL) {
        return 1;
    }

    const int left_value = *(const int *)left;
    const int right_value = *(const int *)right;
    return (left_value > right_value) - (left_value < right_value);
}

int interface_sort_ints(int *values, size_t count) {
    if (values == NULL && count > 0) {
        return 0;
    }

    if (count > 0) {
        qsort(values, count, sizeof *values, interface_compare_int);
    }
    return 1;
}

int interface_find_int(const int *values, size_t count, int target, size_t *index) {
    if (index == NULL) {
        return 0;
    }

    *index = SIZE_MAX;
    if (values == NULL || count == 0) {
        return 0;
    }

    const int *found = bsearch(&target, values, count, sizeof *values,
                               interface_compare_int);
    if (found == NULL) {
        return 0;
    }

    *index = (size_t)(found - values);
    return 1;
}

int interface_average_ints(size_t count, double *output, ...) {
    if (output == NULL) {
        return 0;
    }

    if (count == 0) {
        *output = 0.0;
        return 1;
    }

    va_list arguments;
    va_start(arguments, output);
    double sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        sum += (double)va_arg(arguments, int);
    }
    va_end(arguments);

    *output = sum / (double)count;
    return 1;
}
