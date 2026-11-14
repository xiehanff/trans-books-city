#include "test_common.h"

#include <stdint.h>
#include <string.h>

#include "string_builder.h"

void test_string_builder(void) {
    string_builder *builder = sb_new(1);
    TEST_ASSERT(builder != NULL);
    TEST_ASSERT(sb_len(builder) == 0);
    TEST_ASSERT(sb_capacity(builder) == 1);
    TEST_ASSERT(strcmp(sb_str(builder), "") == 0);

    TEST_ASSERT(sb_append(builder, "hello"));
    TEST_ASSERT(sb_append_len(builder, " world!ignored", 7));
    TEST_ASSERT(sb_append_char(builder, '!'));
    TEST_ASSERT(strcmp(sb_str(builder), "hello world!!") == 0);
    TEST_ASSERT(sb_len(builder) == strlen("hello world!!"));

    TEST_ASSERT(sb_append_fmt(builder, " value=%d", 42));
    TEST_ASSERT(strcmp(sb_str(builder), "hello world!! value=42") == 0);
    const size_t old_capacity = sb_capacity(builder);
    TEST_ASSERT(sb_reserve(builder, old_capacity + 100));
    TEST_ASSERT(sb_capacity(builder) >= old_capacity + 100);
    TEST_ASSERT(sb_reserve(builder, old_capacity) == 1);

    sb_clear(builder);
    TEST_ASSERT(sb_len(builder) == 0 && strcmp(sb_str(builder), "") == 0);
    TEST_ASSERT(sb_append(builder, "reused"));
    TEST_ASSERT(strcmp(sb_str(builder), "reused") == 0);
    TEST_ASSERT(sb_append_len(builder, sb_str(builder), sb_len(builder)));
    TEST_ASSERT(strcmp(sb_str(builder), "reusedreused") == 0);

    TEST_ASSERT(sb_append(NULL, "x") == 0);
    TEST_ASSERT(sb_append_len(builder, NULL, 1) == 0);
    TEST_ASSERT(sb_append_len(builder, "", 0) == 1);
    TEST_ASSERT(sb_append_fmt(NULL, "%d", 1) == 0);
    TEST_ASSERT(sb_append_fmt(builder, NULL) == 0);
    TEST_ASSERT(sb_reserve(NULL, 1) == 0);
    TEST_ASSERT(sb_str(NULL)[0] == '\0');
    TEST_ASSERT(sb_len(NULL) == 0 && sb_capacity(NULL) == 0);

    const size_t saved_length = builder->length;
    builder->length = SIZE_MAX;
    TEST_ASSERT(sb_append_len(builder, "x", 1) == 0);
    builder->length = saved_length;
    sb_free(builder);
    sb_free(NULL);
}
