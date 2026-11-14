#include "string_builder.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SB_DEFAULT_CAPACITY 64U
#define SB_GROWTH_STEP (1024U * 1024U)

static int checked_required_capacity(const string_builder *builder,
                                     size_t appended_length,
                                     size_t *required) {
    if (builder->length > SIZE_MAX - appended_length ||
        builder->length + appended_length == SIZE_MAX) {
        return 0;
    }
    *required = builder->length + appended_length + 1;
    return 1;
}

static int next_capacity(size_t current, size_t required, size_t *result) {
    size_t candidate;
    if (current == 0) {
        candidate = SB_DEFAULT_CAPACITY;
    } else if (current > SIZE_MAX / 2) {
        candidate = SIZE_MAX;
    } else {
        candidate = current * 2;
    }

    if (candidate > SB_GROWTH_STEP) {
        if (current > SIZE_MAX - SB_GROWTH_STEP) {
            candidate = SIZE_MAX;
        } else {
            candidate = current + SB_GROWTH_STEP;
        }
    }
    if (candidate < required) {
        candidate = required;
    }
    if (candidate < current) {
        return 0;
    }
    *result = candidate;
    return 1;
}

int sb_reserve(string_builder *builder, size_t new_capacity) {
    if (builder == NULL || new_capacity <= builder->capacity ||
        new_capacity == 0) {
        return builder != NULL;
    }
    char *new_data = realloc(builder->data, new_capacity);
    if (new_data == NULL) {
        return 0;
    }
    builder->data = new_data;
    builder->capacity = new_capacity;
    if (builder->length == 0) {
        builder->data[0] = '\0';
    }
    return 1;
}

string_builder *sb_new(size_t initial_capacity) {
    string_builder *builder = malloc(sizeof *builder);
    if (builder == NULL) {
        return NULL;
    }
    builder->data = NULL;
    builder->length = 0;
    builder->capacity = 0;

    const size_t capacity = initial_capacity == 0 ? SB_DEFAULT_CAPACITY
                                                  : initial_capacity;
    if (!sb_reserve(builder, capacity)) {
        free(builder);
        return NULL;
    }
    builder->data[0] = '\0';
    return builder;
}

void sb_free(string_builder *builder) {
    if (builder == NULL) {
        return;
    }
    free(builder->data);
    free(builder);
}

int sb_append_len(string_builder *builder, const char *text, size_t length) {
    if (builder == NULL || text == NULL) {
        return 0;
    }
    if (length == 0) {
        return 1;
    }

    /* sb_str(builder) 可能在扩容时失效，先复制这个公开的内部别名。 */
    if (text == builder->data) {
        char *copy = malloc(length);
        if (copy == NULL) {
            return 0;
        }
        memcpy(copy, text, length);
        const int result = sb_append_len(builder, copy, length);
        free(copy);
        return result;
    }

    size_t required = 0;
    if (!checked_required_capacity(builder, length, &required)) {
        return 0;
    }
    if (required > builder->capacity) {
        size_t capacity = 0;
        if (!next_capacity(builder->capacity, required, &capacity) ||
            !sb_reserve(builder, capacity)) {
            return 0;
        }
    }
    memcpy(builder->data + builder->length, text, length);
    builder->length += length;
    builder->data[builder->length] = '\0';
    return 1;
}

int sb_append(string_builder *builder, const char *text) {
    if (builder == NULL || text == NULL) {
        return 0;
    }
    return sb_append_len(builder, text, strlen(text));
}

int sb_append_char(string_builder *builder, char character) {
    return sb_append_len(builder, &character, 1);
}

static int sb_append_fmt_placeholder(string_builder *builder,
                                      const char *format, int needed,
                                      va_list arguments);

int sb_append_fmt(string_builder *builder, const char *format, ...) {
    if (builder == NULL || format == NULL) {
        return 0;
    }

    va_list arguments;
    va_start(arguments, format);
    va_list copy;
    va_copy(copy, arguments);
    const int needed = vsnprintf(NULL, 0, format, arguments);
    va_end(arguments);
    if (needed < 0) {
        va_end(copy);
        return 0;
    }

    const int result = sb_append_fmt_placeholder(builder, format, needed, copy);
    va_end(copy);
    return result;
}

/* The va_list is consumed by this helper after capacity has been checked. */
static int sb_append_fmt_placeholder(string_builder *builder,
                                      const char *format, int needed,
                                      va_list arguments) {
    const size_t length = (size_t)needed;
    size_t required = 0;
    if (!checked_required_capacity(builder, length, &required)) {
        return 0;
    }
    if (required > builder->capacity) {
        size_t capacity = 0;
        if (!next_capacity(builder->capacity, required, &capacity) ||
            !sb_reserve(builder, capacity)) {
            return 0;
        }
    }
    const int written = vsnprintf(builder->data + builder->length,
                                  length + 1, format, arguments);
    if (written != needed) {
        return 0;
    }
    builder->length += length;
    return 1;
}

void sb_clear(string_builder *builder) {
    if (builder == NULL) {
        return;
    }
    builder->length = 0;
    if (builder->data != NULL) {
        builder->data[0] = '\0';
    }
}

const char *sb_str(const string_builder *builder) {
    if (builder == NULL || builder->data == NULL) {
        return "";
    }
    return builder->data;
}

size_t sb_len(const string_builder *builder) {
    return builder == NULL ? 0 : builder->length;
}

size_t sb_capacity(const string_builder *builder) {
    return builder == NULL ? 0 : builder->capacity;
}
