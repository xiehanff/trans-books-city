#include "function_pointer.h"

#include <limits.h>
#include <stdlib.h>

static int saturating_add(int left, int right) {
    if (right > 0 && left > INT_MAX - right) {
        return INT_MAX;
    }
    if (right < 0 && left < INT_MIN - right) {
        return INT_MIN;
    }
    return left + right;
}

static int saturating_subtract(int left, int right) {
    if (right > 0 && left < INT_MIN + right) {
        return INT_MIN;
    }
    if (right < 0 && left > INT_MAX + right) {
        return INT_MAX;
    }
    return left - right;
}

static int saturating_multiply(int left, int right) {
    if (left == 0 || right == 0) {
        return 0;
    }
    if (left == -1) {
        return right == INT_MIN ? INT_MAX : -right;
    }
    if (right == -1) {
        return left == INT_MIN ? INT_MAX : -left;
    }

    if (left > 0) {
        if (right > 0 && left > INT_MAX / right) {
            return INT_MAX;
        }
        if (right < 0 && right < INT_MIN / left) {
            return INT_MIN;
        }
    } else if (right > 0) {
        if (left < INT_MIN / right) {
            return INT_MIN;
        }
    } else if (left < INT_MAX / right) {
        return INT_MAX;
    }
    return left * right;
}

int add(int left, int right) {
    return saturating_add(left, right);
}

int subtract(int left, int right) {
    return saturating_subtract(left, right);
}

int multiply(int left, int right) {
    return saturating_multiply(left, right);
}

int apply_operation(BinaryOperation operation, int left, int right, int *result) {
    if (operation == NULL || result == NULL) {
        return 0;
    }

    *result = operation(left, right);
    return 1;
}

void visit_ints(int *values, size_t count, IntVisitor visitor, void *context) {
    if (visitor == NULL || (values == NULL && count > 0)) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        visitor(&values[i], context);
    }
}

int compare_int_ascending(const void *left, const void *right) {
    const int left_value = *(const int *)left;
    const int right_value = *(const int *)right;
    return (left_value > right_value) - (left_value < right_value);
}

int *create_int(int value) {
    int *result = malloc(sizeof *result);
    if (result == NULL) {
        return NULL;
    }

    *result = value;
    return result;
}

