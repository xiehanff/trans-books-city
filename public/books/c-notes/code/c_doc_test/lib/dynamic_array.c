#include "dynamic_array.h"

#include <stdint.h>
#include <stdlib.h>

void int_array_init(IntArray *array) {
    if (array == NULL) {
        return;
    }

    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
}

int int_array_reserve(IntArray *array, size_t minimum_capacity) {
    if (array == NULL || minimum_capacity <= array->capacity) {
        return array != NULL;
    }

    if (minimum_capacity > SIZE_MAX / sizeof *array->data) {
        return 0;
    }

    int *new_data = realloc(array->data, minimum_capacity * sizeof *array->data);
    if (new_data == NULL) {
        return 0;
    }

    array->data = new_data;
    array->capacity = minimum_capacity;
    return 1;
}

int int_array_push(IntArray *array, int value) {
    if (array == NULL || array->size == SIZE_MAX) {
        return 0;
    }

    if (array->size == array->capacity) {
        size_t new_capacity = array->capacity == 0 ? 4 : array->capacity;
        if (new_capacity > SIZE_MAX / 2) {
            return 0;
        }
        new_capacity *= 2;
        if (!int_array_reserve(array, new_capacity)) {
            return 0;
        }
    }

    array->data[array->size] = value;
    array->size++;
    return 1;
}

int int_array_get(const IntArray *array, size_t index, int *value) {
    if (array == NULL || value == NULL || index >= array->size) {
        return 0;
    }

    *value = array->data[index];
    return 1;
}

int int_array_pop(IntArray *array, int *value) {
    if (array == NULL || array->size == 0) {
        return 0;
    }

    array->size--;
    if (value != NULL) {
        *value = array->data[array->size];
    }
    return 1;
}

void int_array_free(IntArray *array) {
    if (array == NULL) {
        return;
    }

    free(array->data);
    int_array_init(array);
}

