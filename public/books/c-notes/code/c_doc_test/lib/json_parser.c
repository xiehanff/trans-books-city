#include "json_parser.h"

#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *input;
    size_t position;
    size_t length;
} json_context;

static void json_destroy_contents(json_value *value);

static int has_characters(const json_context *context, size_t count) {
    return count <= context->length - context->position;
}

static char current_character(const json_context *context) {
    if (context->position >= context->length) {
        return '\0';
    }
    return context->input[context->position];
}

static void skip_whitespace(json_context *context) {
    while (context->position < context->length) {
        unsigned char character = (unsigned char)context->input[context->position];
        if (character != ' ' && character != '\t' && character != '\n' &&
            character != '\r') {
            break;
        }
        context->position++;
    }
}

static int parse_literal(json_context *context, const char *literal,
                         size_t length) {
    if (!has_characters(context, length) ||
        memcmp(context->input + context->position, literal, length) != 0) {
        return 0;
    }
    context->position += length;
    return 1;
}

static int parse_null(json_context *context, json_value *value) {
    if (!parse_literal(context, "null", 4)) {
        return 0;
    }
    value->type = JSON_NULL;
    return 1;
}

static int parse_bool(json_context *context, json_value *value) {
    if (parse_literal(context, "true", 4)) {
        value->type = JSON_BOOL;
        value->data.boolean = 1;
        return 1;
    }
    if (parse_literal(context, "false", 5)) {
        value->type = JSON_BOOL;
        value->data.boolean = 0;
        return 1;
    }
    return 0;
}

static int parse_number(json_context *context, json_value *value) {
    const size_t start = context->position;

    if (current_character(context) == '-') {
        context->position++;
    }

    if (current_character(context) == '0') {
        context->position++;
    } else if (isdigit((unsigned char)current_character(context))) {
        do {
            context->position++;
        } while (isdigit((unsigned char)current_character(context)));
    } else {
        return 0;
    }

    if (current_character(context) == '.') {
        context->position++;
        if (!isdigit((unsigned char)current_character(context))) {
            return 0;
        }
        do {
            context->position++;
        } while (isdigit((unsigned char)current_character(context)));
    }

    if (current_character(context) == 'e' || current_character(context) == 'E') {
        context->position++;
        if (current_character(context) == '+' || current_character(context) == '-') {
            context->position++;
        }
        if (!isdigit((unsigned char)current_character(context))) {
            return 0;
        }
        do {
            context->position++;
        } while (isdigit((unsigned char)current_character(context)));
    }

    const size_t number_length = context->position - start;
    if (number_length == SIZE_MAX) {
        return 0;
    }
    char *number_text = malloc(number_length + 1);
    if (number_text == NULL) {
        return 0;
    }
    memcpy(number_text, context->input + start, number_length);
    number_text[number_length] = '\0';

    char *end = NULL;
    double number = strtod(number_text, &end);
    const int valid = end == number_text + number_length && isfinite(number);
    free(number_text);
    if (!valid) {
        return 0;
    }

    value->type = JSON_NUMBER;
    value->data.number = number;
    return 1;
}

static int parse_hex4(const char *text) {
    int code = 0;
    for (size_t i = 0; i < 4; i++) {
        const unsigned char character = (unsigned char)text[i];
        code <<= 4;
        if (character >= '0' && character <= '9') {
            code |= (int)(character - '0');
        } else if (character >= 'a' && character <= 'f') {
            code |= (int)(character - 'a' + 10);
        } else if (character >= 'A' && character <= 'F') {
            code |= (int)(character - 'A' + 10);
        } else {
            return -1;
        }
    }
    return code;
}

static size_t encode_utf8(int code, char *output) {
    if (code < 0x80) {
        output[0] = (char)code;
        return 1;
    }
    if (code < 0x800) {
        output[0] = (char)(0xc0 | (code >> 6));
        output[1] = (char)(0x80 | (code & 0x3f));
        return 2;
    }
    if (code < 0x10000) {
        output[0] = (char)(0xe0 | (code >> 12));
        output[1] = (char)(0x80 | ((code >> 6) & 0x3f));
        output[2] = (char)(0x80 | (code & 0x3f));
        return 3;
    }
    output[0] = (char)(0xf0 | (code >> 18));
    output[1] = (char)(0x80 | ((code >> 12) & 0x3f));
    output[2] = (char)(0x80 | ((code >> 6) & 0x3f));
    output[3] = (char)(0x80 | (code & 0x3f));
    return 4;
}

static int parse_string_text(json_context *context, char **result) {
    if (current_character(context) != '"') {
        return 0;
    }
    context->position++;

    const size_t remaining = context->length - context->position;
    if (remaining == SIZE_MAX) {
        return 0;
    }
    char *output = malloc(remaining + 1);
    if (output == NULL) {
        return 0;
    }

    size_t output_length = 0;
    while (context->position < context->length) {
        const unsigned char character =
            (unsigned char)context->input[context->position];
        if (character == '"') {
            context->position++;
            output[output_length] = '\0';
            *result = output;
            return 1;
        }
        if (character < 0x20) {
            free(output);
            return 0;
        }

        if (character != '\\') {
            output[output_length++] = (char)character;
            context->position++;
            continue;
        }

        context->position++;
        if (context->position >= context->length) {
            free(output);
            return 0;
        }
        const char escape = context->input[context->position];
        context->position++;
        switch (escape) {
            case '"': output[output_length++] = '"'; break;
            case '\\': output[output_length++] = '\\'; break;
            case '/': output[output_length++] = '/'; break;
            case 'b': output[output_length++] = '\b'; break;
            case 'f': output[output_length++] = '\f'; break;
            case 'n': output[output_length++] = '\n'; break;
            case 'r': output[output_length++] = '\r'; break;
            case 't': output[output_length++] = '\t'; break;
            case 'u': {
                if (!has_characters(context, 4)) {
                    free(output);
                    return 0;
                }
                const int high = parse_hex4(context->input + context->position);
                if (high < 0) {
                    free(output);
                    return 0;
                }
                context->position += 4;
                int code = high;
                if (high >= 0xd800 && high <= 0xdbff) {
                    if (!has_characters(context, 6) ||
                        context->input[context->position] != '\\' ||
                        context->input[context->position + 1] != 'u') {
                        free(output);
                        return 0;
                    }
                    const int low = parse_hex4(context->input + context->position + 2);
                    if (low < 0 || low < 0xdc00 || low > 0xdfff) {
                        free(output);
                        return 0;
                    }
                    code = 0x10000 + ((high - 0xd800) << 10) + (low - 0xdc00);
                    context->position += 6;
                } else if (high >= 0xdc00 && high <= 0xdfff) {
                    free(output);
                    return 0;
                }
                output_length += encode_utf8(code, output + output_length);
                break;
            }
            default:
                free(output);
                return 0;
        }
    }

    free(output);
    return 0;
}

static void json_destroy_array(json_value *value) {
    for (size_t i = 0; i < value->data.array.count; i++) {
        json_destroy_contents(&value->data.array.items[i]);
    }
    free(value->data.array.items);
}

static void json_destroy_object(json_value *value) {
    for (size_t i = 0; i < value->data.object.count; i++) {
        free(value->data.object.keys[i]);
        json_destroy_contents(&value->data.object.values[i]);
    }
    free(value->data.object.keys);
    free(value->data.object.values);
}

static void json_destroy_contents(json_value *value) {
    if (value == NULL) {
        return;
    }
    switch (value->type) {
        case JSON_STRING:
            free(value->data.string);
            break;
        case JSON_ARRAY:
            json_destroy_array(value);
            break;
        case JSON_OBJECT:
            json_destroy_object(value);
            break;
        default:
            break;
    }
    value->type = JSON_NULL;
}

static int parse_value(json_context *context, json_value *value,
                       size_t depth);

static int grow_array(json_value *value, size_t *capacity) {
    if (*capacity > SIZE_MAX / 2 ||
        *capacity > SIZE_MAX / sizeof *value->data.array.items) {
        return 0;
    }
    const size_t new_capacity = *capacity * 2;
    json_value *items = realloc(value->data.array.items,
                                new_capacity * sizeof *items);
    if (items == NULL) {
        return 0;
    }
    value->data.array.items = items;
    *capacity = new_capacity;
    return 1;
}

static int parse_array(json_context *context, json_value *value,
                       size_t depth) {
    if (current_character(context) != '[' || depth >= JSON_MAX_DEPTH) {
        return 0;
    }
    context->position++;
    value->type = JSON_ARRAY;
    value->data.array.items = NULL;
    value->data.array.count = 0;

    skip_whitespace(context);
    if (current_character(context) == ']') {
        context->position++;
        return 1;
    }

    size_t capacity = 4;
    if (capacity > SIZE_MAX / sizeof *value->data.array.items) {
        json_destroy_contents(value);
        return 0;
    }
    value->data.array.items = malloc(capacity * sizeof *value->data.array.items);
    if (value->data.array.items == NULL) {
        json_destroy_contents(value);
        return 0;
    }

    for (;;) {
        skip_whitespace(context);
        if (value->data.array.count == capacity &&
            !grow_array(value, &capacity)) {
            json_destroy_contents(value);
            return 0;
        }
        json_value *item = &value->data.array.items[value->data.array.count];
        item->type = JSON_NULL;
        if (!parse_value(context, item, depth + 1U)) {
            json_destroy_contents(value);
            return 0;
        }
        value->data.array.count++;

        skip_whitespace(context);
        if (current_character(context) == ']') {
            context->position++;
            return 1;
        }
        if (current_character(context) != ',') {
            json_destroy_contents(value);
            return 0;
        }
        context->position++;
    }
}

static int grow_object(json_value *value, size_t *capacity) {
    if (*capacity > SIZE_MAX / 2) {
        return 0;
    }
    const size_t new_capacity = *capacity * 2;
    if (new_capacity > SIZE_MAX / sizeof *value->data.object.keys ||
        new_capacity > SIZE_MAX / sizeof *value->data.object.values) {
        return 0;
    }
    char **keys = malloc(new_capacity * sizeof *keys);
    json_value *values = malloc(new_capacity * sizeof *values);
    if (keys == NULL || values == NULL) {
        free(keys);
        free(values);
        return 0;
    }
    memcpy(keys, value->data.object.keys,
           value->data.object.count * sizeof *keys);
    memcpy(values, value->data.object.values,
           value->data.object.count * sizeof *values);
    free(value->data.object.keys);
    free(value->data.object.values);
    value->data.object.keys = keys;
    value->data.object.values = values;
    *capacity = new_capacity;
    return 1;
}

static int parse_object(json_context *context, json_value *value,
                        size_t depth) {
    if (current_character(context) != '{' || depth >= JSON_MAX_DEPTH) {
        return 0;
    }
    context->position++;
    value->type = JSON_OBJECT;
    value->data.object.keys = NULL;
    value->data.object.values = NULL;
    value->data.object.count = 0;

    skip_whitespace(context);
    if (current_character(context) == '}') {
        context->position++;
        return 1;
    }

    size_t capacity = 4;
    if (capacity > SIZE_MAX / sizeof *value->data.object.keys ||
        capacity > SIZE_MAX / sizeof *value->data.object.values) {
        json_destroy_contents(value);
        return 0;
    }
    value->data.object.keys = malloc(capacity * sizeof *value->data.object.keys);
    value->data.object.values = malloc(capacity * sizeof *value->data.object.values);
    if (value->data.object.keys == NULL || value->data.object.values == NULL) {
        json_destroy_contents(value);
        return 0;
    }

    for (;;) {
        skip_whitespace(context);
        if (value->data.object.count == capacity &&
            !grow_object(value, &capacity)) {
            json_destroy_contents(value);
            return 0;
        }

        char *key = NULL;
        if (!parse_string_text(context, &key)) {
            json_destroy_contents(value);
            return 0;
        }
        skip_whitespace(context);
        if (current_character(context) != ':') {
            free(key);
            json_destroy_contents(value);
            return 0;
        }
        context->position++;
        skip_whitespace(context);

        const size_t index = value->data.object.count;
        value->data.object.values[index].type = JSON_NULL;
        if (!parse_value(context, &value->data.object.values[index],
                         depth + 1U)) {
            free(key);
            json_destroy_contents(&value->data.object.values[index]);
            json_destroy_contents(value);
            return 0;
        }
        value->data.object.keys[index] = key;
        value->data.object.count++;

        skip_whitespace(context);
        if (current_character(context) == '}') {
            context->position++;
            return 1;
        }
        if (current_character(context) != ',') {
            json_destroy_contents(value);
            return 0;
        }
        context->position++;
    }
}

static int parse_value(json_context *context, json_value *value,
                       size_t depth) {
    skip_whitespace(context);
    value->type = JSON_NULL;
    switch (current_character(context)) {
        case 'n': return parse_null(context, value);
        case 't':
        case 'f': return parse_bool(context, value);
        case '"': {
            char *text = NULL;
            if (!parse_string_text(context, &text)) {
                return 0;
            }
            value->type = JSON_STRING;
            value->data.string = text;
            return 1;
        }
        case '[': return parse_array(context, value, depth);
        case '{': return parse_object(context, value, depth);
        default:
            if (current_character(context) == '-' ||
                isdigit((unsigned char)current_character(context))) {
                return parse_number(context, value);
            }
            return 0;
    }
}

json_value *json_parse(const char *input) {
    if (input == NULL) {
        return NULL;
    }
    json_value *value = malloc(sizeof *value);
    if (value == NULL) {
        return NULL;
    }
    value->type = JSON_NULL;
    json_context context = {input, 0, strlen(input)};
    if (!parse_value(&context, value, 0U)) {
        json_destroy_contents(value);
        free(value);
        return NULL;
    }
    skip_whitespace(&context);
    if (context.position != context.length) {
        json_destroy_contents(value);
        free(value);
        return NULL;
    }
    return value;
}

void json_free(json_value *value) {
    if (value == NULL) {
        return;
    }
    json_destroy_contents(value);
    free(value);
}

json_type json_get_type(const json_value *value) {
    return value == NULL ? JSON_NULL : value->type;
}

int json_is_null(const json_value *value) { return value != NULL && value->type == JSON_NULL; }
int json_is_bool(const json_value *value) { return value != NULL && value->type == JSON_BOOL; }
int json_is_number(const json_value *value) { return value != NULL && value->type == JSON_NUMBER; }
int json_is_string(const json_value *value) { return value != NULL && value->type == JSON_STRING; }
int json_is_array(const json_value *value) { return value != NULL && value->type == JSON_ARRAY; }
int json_is_object(const json_value *value) { return value != NULL && value->type == JSON_OBJECT; }

int json_get_bool(const json_value *value) {
    return json_is_bool(value) ? value->data.boolean : 0;
}

double json_get_number(const json_value *value) {
    return json_is_number(value) ? value->data.number : 0.0;
}

const char *json_get_string(const json_value *value) {
    return json_is_string(value) ? value->data.string : NULL;
}

size_t json_array_size(const json_value *array) {
    return json_is_array(array) ? array->data.array.count : 0;
}

json_value *json_array_get(json_value *array, size_t index) {
    if (!json_is_array(array) || index >= array->data.array.count) {
        return NULL;
    }
    return &array->data.array.items[index];
}

const json_value *json_array_get_const(const json_value *array, size_t index) {
    if (!json_is_array(array) || index >= array->data.array.count) {
        return NULL;
    }
    return &array->data.array.items[index];
}

size_t json_object_size(const json_value *object) {
    return json_is_object(object) ? object->data.object.count : 0;
}

json_value *json_object_get(json_value *object, const char *key) {
    if (!json_is_object(object) || key == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < object->data.object.count; i++) {
        if (strcmp(object->data.object.keys[i], key) == 0) {
            return &object->data.object.values[i];
        }
    }
    return NULL;
}

const json_value *json_object_get_const(const json_value *object, const char *key) {
    if (!json_is_object(object) || key == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < object->data.object.count; i++) {
        if (strcmp(object->data.object.keys[i], key) == 0) {
            return &object->data.object.values[i];
        }
    }
    return NULL;
}

const char *json_object_key(const json_value *object, size_t index) {
    if (!json_is_object(object) || index >= object->data.object.count) {
        return NULL;
    }
    return object->data.object.keys[index];
}

json_value *json_object_value(json_value *object, size_t index) {
    if (!json_is_object(object) || index >= object->data.object.count) {
        return NULL;
    }
    return &object->data.object.values[index];
}

const json_value *json_object_value_const(const json_value *object, size_t index) {
    if (!json_is_object(object) || index >= object->data.object.count) {
        return NULL;
    }
    return &object->data.object.values[index];
}
