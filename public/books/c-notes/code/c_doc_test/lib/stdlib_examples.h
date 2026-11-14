#ifndef C_DOC_TEST_STDLIB_EXAMPLES_H
#define C_DOC_TEST_STDLIB_EXAMPLES_H

#include <stddef.h>

/* bytes 必须同时满足源、目标非空以及目标容量约束；bytes 为零时允许空指针。 */
int stdlib_copy_bytes(void *destination, size_t destination_capacity,
                      const void *source, size_t bytes);
int stdlib_move_bytes(void *destination, size_t destination_capacity,
                      const void *source, size_t bytes);
int stdlib_fill_bytes(void *buffer, size_t buffer_capacity, int byte_value,
                      size_t bytes);
int stdlib_compare_bytes(const void *left, const void *right, size_t bytes,
                         int *result);

/* 字符串目标始终以 '\0' 结尾；目标容量包含结尾的 '\0'。 */
int stdlib_string_length(const char *string, size_t capacity, size_t *length);
int stdlib_copy_string(char *destination, size_t destination_capacity,
                       const char *source);
int stdlib_append_string(char *destination, size_t destination_capacity,
                         const char *suffix);
int stdlib_duplicate_string(const char *source, char **copy);

/* 分配函数统一使用输出参数，失败时不丢失原指针；calloc 的输出须先为 NULL。 */
int stdlib_calloc_array(size_t count, size_t element_size, void **memory);
int stdlib_realloc_array(void **memory, size_t count, size_t element_size);
void stdlib_free(void **memory);

#endif
