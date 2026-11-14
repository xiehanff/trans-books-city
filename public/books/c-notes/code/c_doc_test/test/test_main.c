#include <stddef.h>
#include <stdio.h>
#include <string.h>

void test_architecture_examples(void);
void test_basic_examples(void);
void test_bit_operations(void);
void test_control_flow(void);
void test_dynamic_array(void);
void test_function_pointer(void);
void test_generic_vector(void);
void test_http_protocol(void);
void test_interface_examples(void);
void test_json_parser(void);
void test_linked_list(void);
void test_memory_pool(void);
void test_number_examples(void);
void test_sort_search(void);
void test_stack_queue(void);
void test_stdlib_examples(void);
void test_string_builder(void);
void test_string_examples(void);
void test_struct_array(void);

typedef void (*TestFunction)(void);

typedef struct {
    const char *name;
    TestFunction function;
} TestCase;

static const TestCase TEST_CASES[] = {
    {"architecture_examples", test_architecture_examples},
    {"basic_examples", test_basic_examples},
    {"bit_operations", test_bit_operations},
    {"control_flow", test_control_flow},
    {"dynamic_array", test_dynamic_array},
    {"function_pointer", test_function_pointer},
    {"generic_vector", test_generic_vector},
    {"http_protocol", test_http_protocol},
    {"interface_examples", test_interface_examples},
    {"json_parser", test_json_parser},
    {"linked_list", test_linked_list},
    {"memory_pool", test_memory_pool},
    {"number_examples", test_number_examples},
    {"sort_search", test_sort_search},
    {"stack_queue", test_stack_queue},
    {"stdlib_examples", test_stdlib_examples},
    {"string_builder", test_string_builder},
    {"string_examples", test_string_examples},
    {"struct_array", test_struct_array},
};

static void run_all_tests(void) {
    for (size_t i = 0; i < sizeof TEST_CASES / sizeof TEST_CASES[0]; i++) {
        TEST_CASES[i].function();
        printf("[通过] %s\n", TEST_CASES[i].name);
    }
}

static int run_named_test(const char *name) {
    for (size_t i = 0; i < sizeof TEST_CASES / sizeof TEST_CASES[0]; i++) {
        if (strcmp(name, TEST_CASES[i].name) == 0) {
            TEST_CASES[i].function();
            printf("[通过] %s\n", TEST_CASES[i].name);
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        run_all_tests();
    } else if (argc == 2 && run_named_test(argv[1])) {
        /* 通过命令行参数运行单个专题，便于 CTest 定位失败范围。 */
    } else {
        fprintf(stderr,
                "用法: %s [architecture_examples|basic_examples|"
                "bit_operations|control_flow|dynamic_array|function_pointer|"
                "generic_vector|http_protocol|interface_examples|json_parser|linked_list|"
                "memory_pool|number_examples|sort_search|stack_queue|"
                "stdlib_examples|string_builder|string_examples|struct_array]\n",
                argv[0]);
        return 2;
    }

    puts("本次 C 笔记代码测试通过");
    return 0;
}
