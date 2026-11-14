#ifndef C_DOC_TEST_INTERFACE_EXAMPLES_H
#define C_DOC_TEST_INTERFACE_EXAMPLES_H

#include <stddef.h>

/* 只读参数、输出参数和多级指针都在接口层明确表达。 */
int interface_set_int(int *value, int replacement);
int interface_read_int(const int *value, int *output);
int interface_set_int_pointer(int **slot, int *value);
int interface_read_pointer_chain(int *const *slot, int *output);
int interface_swap_int(int *left, int *right);

int interface_divide(int dividend, int divisor, int *quotient, int *remainder);
int interface_sum_ints(const int *values, size_t count, int *output);

/* 比较结果只保证小于、等于或大于零，不依赖整数相减不溢出。 */
int interface_compare_int(const void *left, const void *right);
int interface_sort_ints(int *values, size_t count);
int interface_find_int(const int *values, size_t count, int target,
                       size_t *index);

/* count 为零时写入 0.0，不读取省略号参数。其余参数必须是 int。 */
int interface_average_ints(size_t count, double *output, ...);

/* 这些宏只在测试中使用无副作用的普通变量或常量。 */
#define C_DOC_SQUARE(value) ((value) * (value))
#define C_DOC_ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define C_DOC_STRINGIFY_IMPL(value) #value
#define C_DOC_STRINGIFY(value) C_DOC_STRINGIFY_IMPL(value)

#define C_DOC_TOKEN_PASTE_IMPL(left, right) left##right
#define C_DOC_TOKEN_PASTE(left, right) C_DOC_TOKEN_PASTE_IMPL(left, right)

#define C_DOC_SWAP_INT(left, right)                                      \
    do {                                                                 \
        int c_doc_macro_swap_temp = (left);                             \
        (left) = (right);                                                \
        (right) = c_doc_macro_swap_temp;                                 \
    } while (0)

#endif
