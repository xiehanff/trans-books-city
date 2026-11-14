#ifndef C_DOC_TEST_DYNAMIC_ARRAY_H
#define C_DOC_TEST_DYNAMIC_ARRAY_H

#include <stddef.h>

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} IntArray;

void int_array_init(IntArray *array);
int int_array_reserve(IntArray *array, size_t minimum_capacity);
int int_array_push(IntArray *array, int value);
int int_array_get(const IntArray *array, size_t index, int *value);
int int_array_pop(IntArray *array, int *value);
void int_array_free(IntArray *array);

#endif

