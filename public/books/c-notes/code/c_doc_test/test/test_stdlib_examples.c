#include "test_common.h"

#include <stdint.h>
#include <string.h>

#include "stdlib_examples.h"

void test_stdlib_examples(void) {
    const unsigned char source[] = {1, 2, 3, 4};
    unsigned char destination[sizeof source] = {0};
    TEST_ASSERT(stdlib_copy_bytes(destination, sizeof destination, source,
                                   sizeof source));
    TEST_ASSERT(memcmp(destination, source, sizeof source) == 0);
    TEST_ASSERT(stdlib_copy_bytes(NULL, 0, NULL, 0));
    TEST_ASSERT(!stdlib_copy_bytes(destination, 2, source, sizeof source));
    TEST_ASSERT(!stdlib_copy_bytes(destination, sizeof destination, NULL, 1));
    TEST_ASSERT(!stdlib_copy_bytes(NULL, sizeof destination, source, 1));

    char moved[] = "abcdef";
    TEST_ASSERT(stdlib_move_bytes(moved + 2, sizeof moved - 2, moved, 4));
    TEST_ASSERT(strcmp(moved, "ababcd") == 0);
    TEST_ASSERT(stdlib_move_bytes(NULL, 0, NULL, 0));
    TEST_ASSERT(!stdlib_move_bytes(moved, 2, moved + 1, 3));

    unsigned char filled[5] = {0};
    TEST_ASSERT(stdlib_fill_bytes(filled, sizeof filled, 0xA5, sizeof filled));
    for (size_t i = 0; i < sizeof filled; i++) {
        TEST_ASSERT(filled[i] == 0xA5);
    }
    TEST_ASSERT(stdlib_fill_bytes(NULL, 0, 0, 0));
    TEST_ASSERT(!stdlib_fill_bytes(filled, sizeof filled, 0, sizeof filled + 1));
    TEST_ASSERT(!stdlib_fill_bytes(NULL, sizeof filled, 0, 1));

    int comparison = 0;
    const unsigned char smaller[] = {1, 2, 3};
    const unsigned char larger[] = {1, 2, 4};
    TEST_ASSERT(stdlib_compare_bytes(smaller, larger, sizeof smaller,
                                     &comparison));
    TEST_ASSERT(comparison < 0);
    TEST_ASSERT(stdlib_compare_bytes(NULL, NULL, 0, &comparison));
    TEST_ASSERT(comparison == 0);
    TEST_ASSERT(!stdlib_compare_bytes(NULL, larger, 1, &comparison));
    TEST_ASSERT(!stdlib_compare_bytes(smaller, larger, 1, NULL));

    size_t length = 0;
    TEST_ASSERT(stdlib_string_length("hello", 6, &length) && length == 5);
    TEST_ASSERT(!stdlib_string_length("hello", 5, &length));
    TEST_ASSERT(!stdlib_string_length(NULL, 0, &length));
    TEST_ASSERT(!stdlib_string_length("hello", 6, NULL));

    char text[16] = {0};
    TEST_ASSERT(stdlib_copy_string(text, sizeof text, "hello"));
    TEST_ASSERT(strcmp(text, "hello") == 0);
    TEST_ASSERT(stdlib_copy_string(text, sizeof text, ""));
    TEST_ASSERT(text[0] == '\0');
    TEST_ASSERT(!stdlib_copy_string(text, 5, "hello"));
    TEST_ASSERT(!stdlib_copy_string(NULL, sizeof text, "hello"));
    TEST_ASSERT(!stdlib_copy_string(text, sizeof text, NULL));

    TEST_ASSERT(stdlib_copy_string(text, sizeof text, "hello"));
    TEST_ASSERT(stdlib_append_string(text, sizeof text, " world"));
    TEST_ASSERT(strcmp(text, "hello world") == 0);
    TEST_ASSERT(!stdlib_append_string(text, 8, "!"));
    char unterminated[] = {'a', 'b', 'c'};
    TEST_ASSERT(!stdlib_append_string(unterminated, sizeof unterminated, "!"));
    TEST_ASSERT(!stdlib_append_string(NULL, 0, "!"));
    TEST_ASSERT(!stdlib_append_string(text, sizeof text, NULL));

    char *copy = NULL;
    TEST_ASSERT(stdlib_duplicate_string("copy me", &copy));
    TEST_ASSERT(copy != NULL && strcmp(copy, "copy me") == 0);
    void *copy_memory = copy;
    stdlib_free(&copy_memory);
    copy = NULL;
    TEST_ASSERT(copy_memory == NULL && copy == NULL);
    TEST_ASSERT(!stdlib_duplicate_string(NULL, &copy));
    TEST_ASSERT(!stdlib_duplicate_string("copy me", NULL));

    void *memory = NULL;
    TEST_ASSERT(stdlib_calloc_array(3, sizeof(int), &memory));
    TEST_ASSERT(memory != NULL);
    int *numbers = (int *)memory;
    TEST_ASSERT(numbers[0] == 0 && numbers[1] == 0 && numbers[2] == 0);
    void *before_second_calloc = memory;
    TEST_ASSERT(!stdlib_calloc_array(1, sizeof(int), &memory));
    TEST_ASSERT(memory == before_second_calloc);
    numbers[0] = 10;
    numbers[1] = 20;
    numbers[2] = 30;

    TEST_ASSERT(stdlib_realloc_array(&memory, 6, sizeof(int)));
    numbers = (int *)memory;
    TEST_ASSERT(numbers[0] == 10 && numbers[1] == 20 && numbers[2] == 30);
    void *before_failed_resize = memory;
    TEST_ASSERT(!stdlib_realloc_array(&memory, SIZE_MAX, 2));
    TEST_ASSERT(memory == before_failed_resize);
    numbers = (int *)memory;
    TEST_ASSERT(numbers[0] == 10 && numbers[1] == 20 && numbers[2] == 30);

    stdlib_free(&memory);
    TEST_ASSERT(memory == NULL);
    stdlib_free(&memory);
    TEST_ASSERT(stdlib_realloc_array(&memory, 0, sizeof(int)) && memory == NULL);
    TEST_ASSERT(stdlib_realloc_array(&memory, 2, sizeof(int)));
    stdlib_free(&memory);
    TEST_ASSERT(!stdlib_realloc_array(NULL, 1, sizeof(int)));
    TEST_ASSERT(!stdlib_calloc_array(SIZE_MAX, 2, &memory));
    TEST_ASSERT(memory == NULL);
    TEST_ASSERT(stdlib_calloc_array(0, sizeof(int), &memory) && memory == NULL);
}
