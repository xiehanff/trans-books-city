#ifndef C_DOC_BASIC_EXAMPLES_H
#define C_DOC_BASIC_EXAMPLES_H

#include <stddef.h>

/* 基础语法中可独立验证的安全示例。 */
typedef struct BasicPoint {
    int x;
    int y;
} BasicPoint;

const char *basic_hello_message(void);
int basic_constant_value(void);
double basic_divide_as_double(int numerator, int denominator);
int basic_short_circuit_and(int left, int right, int *rhs_evaluated);
int basic_short_circuit_or(int left, int right, int *rhs_evaluated);
int basic_conditional_max(int left, int right);
int basic_increment_by_value(int value);
void basic_increment_by_pointer(int *value);
unsigned long long basic_factorial(unsigned int n);
long long basic_array_sum(const int *values, size_t count);
int basic_find_index(const int *values, size_t count, int target);
void basic_translate_point(BasicPoint *point, int dx, int dy);
size_t basic_string_length(const char *text);

#endif
