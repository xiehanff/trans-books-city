#include "test_common.h"

#include <string.h>

#include "string_examples.h"

void test_string_examples(void) {
    /* 第10章：字符串是以 '\0' 结尾的字符数组。 */
    char text[32] = "hello";
    TEST_ASSERT(string_length(text) == 5);
    TEST_ASSERT(string_length(NULL) == 0);
    TEST_ASSERT(string_equal(text, "hello"));
    TEST_ASSERT(!string_equal(text, "world"));
    TEST_ASSERT(!string_equal(NULL, text));

    /* 第10章：用容量限制复制和连接，始终保证目标字符串有结束符。 */
    char destination[12];
    TEST_ASSERT(string_copy(destination, sizeof destination, "hello"));
    TEST_ASSERT(strcmp(destination, "hello") == 0);
    TEST_ASSERT(!string_copy(destination, sizeof destination, "hello world!"));
    TEST_ASSERT(destination[sizeof destination - 1] == '\0');
    TEST_ASSERT(!string_copy(destination, 0, "x"));

    TEST_ASSERT(string_copy(destination, sizeof destination, "hello"));
    TEST_ASSERT(string_append(destination, sizeof destination, "!"));
    TEST_ASSERT(strcmp(destination, "hello!") == 0);
    TEST_ASSERT(!string_append(destination, sizeof destination, " too long"));
    TEST_ASSERT(destination[sizeof destination - 1] == '\0');
    TEST_ASSERT(!string_append(NULL, 0, "x"));

    /* 第10章：strchr、strstr 和 strcmp 的安全使用方式。 */
    TEST_ASSERT(string_find_char(text, 'l') == text + 2);
    TEST_ASSERT(string_find_char(text, 'x') == NULL);
    TEST_ASSERT(string_find_substring(text, "ell") == text + 1);
    TEST_ASSERT(string_find_substring(text, "world") == NULL);
    TEST_ASSERT(string_find_substring(NULL, "x") == NULL);

    /* 第10章：sprintf 的安全版本 snprintf 会报告输出是否完整。 */
    char formatted[32];
    TEST_ASSERT(string_format_score(formatted, sizeof formatted, 85));
    TEST_ASSERT(strcmp(formatted, "分数是85分") == 0);
    char short_formatted[8];
    TEST_ASSERT(!string_format_score(short_formatted, sizeof short_formatted, 85));
    TEST_ASSERT(short_formatted[sizeof short_formatted - 1] == '\0');
}
