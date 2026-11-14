#include "stdlib_examples.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int valid_byte_operation(const void *destination, size_t capacity,
                                const void *source, size_t bytes) {
    if (bytes == 0) {
        return 1;
    }
    return destination != NULL && source != NULL && bytes <= capacity;
}

int stdlib_copy_bytes(void *destination, size_t destination_capacity,
                      const void *source, size_t bytes) {
    if (!valid_byte_operation(destination, destination_capacity, source, bytes)) {
        return 0;
    }

    if (bytes > 0) {
        memcpy(destination, source, bytes);
    }
    return 1;
}

int stdlib_move_bytes(void *destination, size_t destination_capacity,
                      const void *source, size_t bytes) {
    if (!valid_byte_operation(destination, destination_capacity, source, bytes)) {
        return 0;
    }

    if (bytes > 0) {
        memmove(destination, source, bytes);
    }
    return 1;
}

int stdlib_fill_bytes(void *buffer, size_t buffer_capacity, int byte_value,
                      size_t bytes) {
    if (bytes > buffer_capacity || (buffer == NULL && bytes > 0)) {
        return 0;
    }

    if (bytes > 0) {
        memset(buffer, byte_value, bytes);
    }
    return 1;
}

int stdlib_compare_bytes(const void *left, const void *right, size_t bytes,
                         int *result) {
    if (result == NULL || (bytes > 0 && (left == NULL || right == NULL))) {
        return 0;
    }

    if (bytes == 0) {
        *result = 0;
    } else {
        *result = memcmp(left, right, bytes);
    }
    return 1;
}

int stdlib_string_length(const char *string, size_t capacity, size_t *length) {
    if (string == NULL || length == NULL) {
        return 0;
    }

    for (size_t i = 0; i < capacity; i++) {
        if (string[i] == '\0') {
            *length = i;
            return 1;
        }
    }
    return 0;
}

int stdlib_copy_string(char *destination, size_t destination_capacity,
                       const char *source) {
    if (destination == NULL || source == NULL || destination_capacity == 0) {
        return 0;
    }

    const size_t source_length = strlen(source);
    if (source_length == SIZE_MAX || source_length >= destination_capacity) {
        return 0;
    }

    memcpy(destination, source, source_length + 1);
    return 1;
}

int stdlib_append_string(char *destination, size_t destination_capacity,
                         const char *suffix) {
    if (destination == NULL || suffix == NULL || destination_capacity == 0) {
        return 0;
    }

    size_t destination_length = 0;
    if (!stdlib_string_length(destination, destination_capacity,
                              &destination_length)) {
        return 0;
    }

    const size_t suffix_length = strlen(suffix);
    if (suffix_length >= destination_capacity - destination_length) {
        return 0;
    }

    memmove(destination + destination_length, suffix, suffix_length + 1);
    return 1;
}

int stdlib_duplicate_string(const char *source, char **copy) {
    if (source == NULL || copy == NULL) {
        return 0;
    }

    *copy = NULL;
    const size_t source_length = strlen(source);
    if (source_length == SIZE_MAX) {
        return 0;
    }

    char *result = malloc(source_length + 1);
    if (result == NULL) {
        return 0;
    }

    memcpy(result, source, source_length + 1);
    *copy = result;
    return 1;
}

int stdlib_calloc_array(size_t count, size_t element_size, void **memory) {
    if (memory == NULL) {
        return 0;
    }

    if (*memory != NULL) {
        return 0;
    }
    if (count == 0 || element_size == 0) {
        return 1;
    }
    if (count > SIZE_MAX / element_size) {
        return 0;
    }

    void *result = calloc(count, element_size);
    if (result == NULL) {
        return 0;
    }

    *memory = result;
    return 1;
}

int stdlib_realloc_array(void **memory, size_t count, size_t element_size) {
    if (memory == NULL) {
        return 0;
    }

    if (count == 0 || element_size == 0) {
        free(*memory);
        *memory = NULL;
        return 1;
    }
    if (count > SIZE_MAX / element_size) {
        return 0;
    }

    void *result = realloc(*memory, count * element_size);
    if (result == NULL) {
        return 0;
    }

    *memory = result;
    return 1;
}

void stdlib_free(void **memory) {
    if (memory == NULL) {
        return;
    }

    free(*memory);
    *memory = NULL;
}
