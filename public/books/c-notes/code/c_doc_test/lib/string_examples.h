#ifndef C_DOC_TEST_STRING_EXAMPLES_H
#define C_DOC_TEST_STRING_EXAMPLES_H

#include <stddef.h>

size_t string_length(const char *string);
int string_copy(char *destination, size_t destination_size,
                const char *source);
int string_append(char *destination, size_t destination_size,
                  const char *source);
const char *string_find_char(const char *string, int character);
const char *string_find_substring(const char *string, const char *substring);
int string_equal(const char *left, const char *right);
int string_format_score(char *destination, size_t destination_size, int score);

#endif
