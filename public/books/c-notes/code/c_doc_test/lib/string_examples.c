#include "string_examples.h"

#include <stdio.h>
#include <string.h>

size_t string_length(const char *string) {
    return string == NULL ? 0 : strlen(string);
}

int string_copy(char *destination, size_t destination_size,
                const char *source) {
    if (destination == NULL || destination_size == 0 || source == NULL) {
        return 0;
    }

    size_t source_length = strlen(source);
    size_t copy_length = source_length < destination_size - 1
                             ? source_length
                             : destination_size - 1;
    memmove(destination, source, copy_length);
    destination[copy_length] = '\0';
    return copy_length == source_length;
}

int string_append(char *destination, size_t destination_size,
                  const char *source) {
    if (destination == NULL || destination_size == 0 || source == NULL) {
        return 0;
    }

    size_t destination_length = 0;
    while (destination_length < destination_size &&
           destination[destination_length] != '\0') {
        destination_length++;
    }
    if (destination_length == destination_size) {
        return 0;
    }

    size_t source_length = strlen(source);
    size_t available = destination_size - destination_length - 1;
    size_t copy_length = source_length < available ? source_length : available;
    memmove(destination + destination_length, source, copy_length);
    destination[destination_length + copy_length] = '\0';
    return copy_length == source_length;
}

const char *string_find_char(const char *string, int character) {
    return string == NULL ? NULL : strchr(string, character);
}

const char *string_find_substring(const char *string, const char *substring) {
    if (string == NULL || substring == NULL) {
        return NULL;
    }
    return strstr(string, substring);
}

int string_equal(const char *left, const char *right) {
    return left != NULL && right != NULL && strcmp(left, right) == 0;
}

int string_format_score(char *destination, size_t destination_size, int score) {
    if (destination == NULL || destination_size == 0) {
        return 0;
    }

    int written = snprintf(destination, destination_size, "分数是%d分", score);
    return written >= 0 && (size_t)written < destination_size;
}
