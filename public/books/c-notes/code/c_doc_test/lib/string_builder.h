#ifndef C_DOC_TEST_STRING_BUILDER_H
#define C_DOC_TEST_STRING_BUILDER_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} string_builder;

string_builder *sb_new(size_t initial_capacity);
void sb_free(string_builder *builder);

int sb_reserve(string_builder *builder, size_t new_capacity);
int sb_append(string_builder *builder, const char *text);
int sb_append_len(string_builder *builder, const char *text, size_t length);
int sb_append_char(string_builder *builder, char character);
int sb_append_fmt(string_builder *builder, const char *format, ...);

void sb_clear(string_builder *builder);
const char *sb_str(const string_builder *builder);
size_t sb_len(const string_builder *builder);
size_t sb_capacity(const string_builder *builder);

#endif
