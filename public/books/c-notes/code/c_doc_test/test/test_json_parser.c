#include "test_common.h"

#include <string.h>

#include "json_parser.h"

void test_json_parser(void) {
    json_value *root = json_parse(
        " { \"name\": \"A\\nB\", \"number\": -12.5e+1, "
        "\"enabled\": true, \"nothing\": null, "
        "\"items\": [false, 0, \"\\u4f60\\u597d\", {\"deep\": 7}], "
        "\"k0\": 0, \"k1\": 1, \"k2\": 2, \"k3\": 3, \"k4\": 4 } ");
    TEST_ASSERT(root != NULL && json_is_object(root));
    TEST_ASSERT(json_object_size(root) == 10);
    TEST_ASSERT(strcmp(json_get_string(json_object_get(root, "name")), "A\nB") == 0);
    TEST_ASSERT(json_get_number(json_object_get(root, "number")) == -125.0);
    TEST_ASSERT(json_get_bool(json_object_get(root, "enabled")) == 1);
    TEST_ASSERT(json_is_null(json_object_get(root, "nothing")));

    json_value *items = json_object_get(root, "items");
    TEST_ASSERT(json_array_size(items) == 4);
    TEST_ASSERT(json_get_bool(json_array_get(items, 0)) == 0);
    TEST_ASSERT(json_get_number(json_array_get(items, 1)) == 0.0);
    TEST_ASSERT(strcmp(json_get_string(json_array_get(items, 2)), "你好") == 0);
    json_value *deep = json_array_get(items, 3);
    TEST_ASSERT(json_get_number(json_object_get(deep, "deep")) == 7.0);
    TEST_ASSERT(json_array_get(items, 4) == NULL);
    TEST_ASSERT(strcmp(json_object_key(root, 0), "name") == 0);
    TEST_ASSERT(json_object_value(root, 10) == NULL);
    TEST_ASSERT(json_object_get(root, "missing") == NULL);
    TEST_ASSERT(json_object_get(root, NULL) == NULL);
    json_free(root);
    json_free(NULL);

    const char *surrogate = "\"\\uD83D\\uDE00\"";
    json_value *emoji = json_parse(surrogate);
    TEST_ASSERT(emoji != NULL && strcmp(json_get_string(emoji), "😀") == 0);
    json_free(emoji);

    const char *invalid[] = {
        NULL, "", "true false", "nullx", "01", "-", "1.", "1e+", 
        "[1,]", "[1 2]", "{\"a\"}", "{\"a\":1,}", "{a:1}",
        "\"bad\\q\"", "\"unterminated", "\"line\nfeed\"",
        "\"\\uD800\"", "\"\\uDE00\"", "NaN"
    };
    for (size_t i = 0; i < sizeof invalid / sizeof invalid[0]; i++) {
        TEST_ASSERT(json_parse(invalid[i]) == NULL);
    }

    char accepted_nested[2U * JSON_MAX_DEPTH + 2U];
    for (size_t i = 0; i < JSON_MAX_DEPTH; i++) {
        accepted_nested[i] = '[';
        accepted_nested[JSON_MAX_DEPTH + 1U + i] = ']';
    }
    accepted_nested[JSON_MAX_DEPTH] = '0';
    accepted_nested[2U * JSON_MAX_DEPTH + 1U] = '\0';
    json_value *nested = json_parse(accepted_nested);
    TEST_ASSERT(nested != NULL);
    json_free(nested);

    char rejected_nested[2U * (JSON_MAX_DEPTH + 1U) + 2U];
    for (size_t i = 0; i < JSON_MAX_DEPTH + 1U; i++) {
        rejected_nested[i] = '[';
        rejected_nested[JSON_MAX_DEPTH + 2U + i] = ']';
    }
    rejected_nested[JSON_MAX_DEPTH + 1U] = '0';
    rejected_nested[2U * (JSON_MAX_DEPTH + 1U) + 1U] = '\0';
    TEST_ASSERT(json_parse(rejected_nested) == NULL);

    json_value scalar;
    scalar.type = JSON_BOOL;
    scalar.data.boolean = 1;
    TEST_ASSERT(json_get_type(&scalar) == JSON_BOOL);
    TEST_ASSERT(json_get_type(NULL) == JSON_NULL);
    TEST_ASSERT(!json_is_number(&scalar));
    TEST_ASSERT(json_get_number(&scalar) == 0.0);
    TEST_ASSERT(json_get_string(&scalar) == NULL);
    TEST_ASSERT(json_array_size(&scalar) == 0);
    TEST_ASSERT(json_object_size(&scalar) == 0);
    TEST_ASSERT(json_object_key(&scalar, 0) == NULL);
}
