#include "basic_examples.h"

#include <limits.h>

const char *basic_hello_message(void)
{
    return "Hello, World!";
}

int basic_constant_value(void)
{
    const int answer = 42;
    const int octal_ten = 012;
    const int hexadecimal_ten = 0xA;

    return answer + octal_ten + hexadecimal_ten;
}

double basic_divide_as_double(int numerator, int denominator)
{
    if (denominator == 0) {
        return 0.0;
    }
    return (double)numerator / (double)denominator;
}

int basic_short_circuit_and(int left, int right, int *rhs_evaluated)
{
    if (rhs_evaluated == NULL) {
        return 0;
    }

    *rhs_evaluated = 0;
    return left != 0 && ((*rhs_evaluated = 1), right != 0);
}

int basic_short_circuit_or(int left, int right, int *rhs_evaluated)
{
    if (rhs_evaluated == NULL) {
        return 0;
    }

    *rhs_evaluated = 0;
    return left != 0 || ((*rhs_evaluated = 1), right != 0);
}

int basic_conditional_max(int left, int right)
{
    return left > right ? left : right;
}

int basic_increment_by_value(int value)
{
    if (value == INT_MAX) {
        return value;
    }
    return value + 1;
}

void basic_increment_by_pointer(int *value)
{
    if (value != NULL && *value != INT_MAX) {
        *value += 1;
    }
}

unsigned long long basic_factorial(unsigned int n)
{
    if (n > 20U) {
        return 0ULL;
    }
    if (n <= 1U) {
        return 1ULL;
    }
    return (unsigned long long)n * basic_factorial(n - 1U);
}

long long basic_array_sum(const int *values, size_t count)
{
    long long total = 0;

    if (values == NULL && count != 0U) {
        return 0;
    }
    for (size_t i = 0U; i < count; ++i) {
        const long long value = values[i];
        if (value > 0 && total > LLONG_MAX - value) {
            return LLONG_MAX;
        }
        if (value < 0 && total < LLONG_MIN - value) {
            return LLONG_MIN;
        }
        total += value;
    }
    return total;
}

int basic_find_index(const int *values, size_t count, int target)
{
    if ((values == NULL && count != 0U) || count > (size_t)INT_MAX) {
        return -1;
    }
    for (size_t i = 0U; i < count; ++i) {
        if (values[i] == target) {
            return (int)i;
        }
    }
    return -1;
}

static int saturating_int_add(int left, int right)
{
    if (right > 0 && left > INT_MAX - right) {
        return INT_MAX;
    }
    if (right < 0 && left < INT_MIN - right) {
        return INT_MIN;
    }
    return left + right;
}

void basic_translate_point(BasicPoint *point, int dx, int dy)
{
    if (point == NULL) {
        return;
    }
    point->x = saturating_int_add(point->x, dx);
    point->y = saturating_int_add(point->y, dy);
}

size_t basic_string_length(const char *text)
{
    size_t length = 0U;

    if (text == NULL) {
        return 0U;
    }
    while (text[length] != '\0') {
        ++length;
    }
    return length;
}
