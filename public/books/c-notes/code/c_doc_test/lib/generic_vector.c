#include "generic_vector.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int vector_bytes(size_t count, size_t element_size, size_t *bytes) {
    if (count > SIZE_MAX / element_size) {
        return 0;
    }
    *bytes = count * element_size;
    return 1;
}

static int vector_ensure_capacity(vector *value, size_t needed) {
    if (needed <= value->capacity) {
        return 1;
    }

    size_t new_capacity = value->capacity == 0 ? 4 : value->capacity;
    while (new_capacity < needed) {
        if (new_capacity > SIZE_MAX / 2) {
            new_capacity = needed;
            break;
        }
        new_capacity *= 2;
    }

    size_t bytes = 0;
    if (!vector_bytes(new_capacity, value->element_size, &bytes)) {
        return 0;
    }
    void *new_data = realloc(value->data, bytes);
    if (new_data == NULL) {
        return 0;
    }
    value->data = new_data;
    value->capacity = new_capacity;
    return 1;
}

vector *vec_create(size_t element_size, size_t initial_capacity) {
    if (element_size == 0) {
        return NULL;
    }
    vector *value = malloc(sizeof *value);
    if (value == NULL) {
        return NULL;
    }
    value->data = NULL;
    value->element_size = element_size;
    value->size = 0;
    value->capacity = 0;

    if (initial_capacity > 0 &&
        !vec_reserve(value, initial_capacity)) {
        free(value);
        return NULL;
    }
    return value;
}

void vec_free(vector *value) {
    if (value == NULL) {
        return;
    }
    free(value->data);
    free(value);
}

int vec_reserve(vector *value, size_t new_capacity) {
    if (value == NULL || new_capacity <= value->capacity) {
        return value != NULL;
    }
    size_t bytes = 0;
    if (!vector_bytes(new_capacity, value->element_size, &bytes)) {
        return 0;
    }
    void *new_data = realloc(value->data, bytes);
    if (new_data == NULL) {
        return 0;
    }
    value->data = new_data;
    value->capacity = new_capacity;
    return 1;
}

int vec_shrink_to_fit(vector *value) {
    if (value == NULL) {
        return 0;
    }
    if (value->size == value->capacity) {
        return 1;
    }
    if (value->size == 0) {
        free(value->data);
        value->data = NULL;
        value->capacity = 0;
        return 1;
    }

    size_t bytes = 0;
    if (!vector_bytes(value->size, value->element_size, &bytes)) {
        return 0;
    }
    void *new_data = realloc(value->data, bytes);
    if (new_data == NULL) {
        return 0;
    }
    value->data = new_data;
    value->capacity = value->size;
    return 1;
}

static void *temporary_copy(const void *element, size_t element_size) {
    void *copy = malloc(element_size);
    if (copy != NULL) {
        memcpy(copy, element, element_size);
    }
    return copy;
}

int vec_push(vector *value, const void *element) {
    if (value == NULL || element == NULL || value->size == SIZE_MAX) {
        return 0;
    }
    void *copy = temporary_copy(element, value->element_size);
    if (copy == NULL) {
        return 0;
    }
    const int success = vector_ensure_capacity(value, value->size + 1);
    if (success) {
        memcpy((unsigned char *)value->data + value->size * value->element_size,
               copy, value->element_size);
        value->size++;
    }
    free(copy);
    return success;
}

int vec_push_n(vector *value, const void *elements, size_t count) {
    if (value == NULL || (elements == NULL && count > 0) ||
        count > SIZE_MAX - value->size) {
        return 0;
    }
    if (count == 0) {
        return 1;
    }

    size_t bytes = 0;
    if (!vector_bytes(count, value->element_size, &bytes)) {
        return 0;
    }
    void *copy = malloc(bytes);
    if (copy == NULL) {
        return 0;
    }
    memcpy(copy, elements, bytes);

    const int success = vector_ensure_capacity(value, value->size + count);
    if (success) {
        memcpy((unsigned char *)value->data + value->size * value->element_size,
               copy, bytes);
        value->size += count;
    }
    free(copy);
    return success;
}

int vec_pop(vector *value, void *element) {
    if (value == NULL || value->size == 0) {
        return 0;
    }
    value->size--;
    if (element != NULL) {
        memcpy(element,
               (unsigned char *)value->data + value->size * value->element_size,
               value->element_size);
    }
    return 1;
}

int vec_insert(vector *value, size_t index, const void *element) {
    if (value == NULL || element == NULL || index > value->size ||
        value->size == SIZE_MAX) {
        return 0;
    }
    void *copy = temporary_copy(element, value->element_size);
    if (copy == NULL) {
        return 0;
    }
    const int success = vector_ensure_capacity(value, value->size + 1);
    if (!success) {
        free(copy);
        return 0;
    }

    unsigned char *base = value->data;
    const size_t moved = value->size - index;
    if (moved > 0) {
        memmove(base + (index + 1) * value->element_size,
                base + index * value->element_size,
                moved * value->element_size);
    }
    memcpy(base + index * value->element_size, copy, value->element_size);
    value->size++;
    free(copy);
    return 1;
}

int vec_erase(vector *value, size_t index) {
    if (value == NULL || index >= value->size) {
        return 0;
    }
    const size_t moved = value->size - index - 1;
    unsigned char *base = value->data;
    if (moved > 0) {
        memmove(base + index * value->element_size,
                base + (index + 1) * value->element_size,
                moved * value->element_size);
    }
    value->size--;
    return 1;
}

void vec_clear(vector *value) {
    if (value != NULL) {
        value->size = 0;
    }
}

void *vec_get(vector *value, size_t index) {
    if (value == NULL || index >= value->size) {
        return NULL;
    }
    return (unsigned char *)value->data + index * value->element_size;
}

const void *vec_get_const(const vector *value, size_t index) {
    if (value == NULL || index >= value->size) {
        return NULL;
    }
    return (const unsigned char *)value->data + index * value->element_size;
}

int vec_set(vector *value, size_t index, const void *element) {
    if (value == NULL || element == NULL || index >= value->size) {
        return 0;
    }
    memmove((unsigned char *)value->data + index * value->element_size,
            element, value->element_size);
    return 1;
}
