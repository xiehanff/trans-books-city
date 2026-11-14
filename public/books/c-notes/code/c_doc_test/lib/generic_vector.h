#ifndef C_DOC_TEST_GENERIC_VECTOR_H
#define C_DOC_TEST_GENERIC_VECTOR_H

#include <stddef.h>

typedef struct {
    void *data;
    size_t element_size;
    size_t size;
    size_t capacity;
} vector;

vector *vec_create(size_t element_size, size_t initial_capacity);
void vec_free(vector *value);

int vec_reserve(vector *value, size_t new_capacity);
int vec_shrink_to_fit(vector *value);
int vec_push(vector *value, const void *element);
int vec_push_n(vector *value, const void *elements, size_t count);
int vec_pop(vector *value, void *element);
int vec_insert(vector *value, size_t index, const void *element);
int vec_erase(vector *value, size_t index);
void vec_clear(vector *value);

void *vec_get(vector *value, size_t index);
const void *vec_get_const(const vector *value, size_t index);
int vec_set(vector *value, size_t index, const void *element);

#endif
