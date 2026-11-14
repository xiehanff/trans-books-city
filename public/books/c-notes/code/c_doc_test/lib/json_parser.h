#ifndef C_DOC_TEST_JSON_PARSER_H
#define C_DOC_TEST_JSON_PARSER_H

#include <stddef.h>

/* 限制递归深度，避免恶意嵌套输入耗尽调用栈。 */
#define JSON_MAX_DEPTH 128U

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} json_type;

typedef struct json_value {
    json_type type;
    union {
        int boolean;
        double number;
        char *string;
        struct {
            struct json_value *items;
            size_t count;
        } array;
        struct {
            char **keys;
            struct json_value *values;
            size_t count;
        } object;
    } data;
} json_value;

json_value *json_parse(const char *input);
void json_free(json_value *value);

json_type json_get_type(const json_value *value);
int json_is_null(const json_value *value);
int json_is_bool(const json_value *value);
int json_is_number(const json_value *value);
int json_is_string(const json_value *value);
int json_is_array(const json_value *value);
int json_is_object(const json_value *value);

int json_get_bool(const json_value *value);
double json_get_number(const json_value *value);
const char *json_get_string(const json_value *value);

size_t json_array_size(const json_value *array);
json_value *json_array_get(json_value *array, size_t index);
const json_value *json_array_get_const(const json_value *array, size_t index);

size_t json_object_size(const json_value *object);
json_value *json_object_get(json_value *object, const char *key);
const json_value *json_object_get_const(const json_value *object, const char *key);
const char *json_object_key(const json_value *object, size_t index);
json_value *json_object_value(json_value *object, size_t index);
const json_value *json_object_value_const(const json_value *object, size_t index);

#endif
