#ifndef C_DOC_TEST_FUNCTION_POINTER_H
#define C_DOC_TEST_FUNCTION_POINTER_H

#include <stddef.h>

typedef int (*BinaryOperation)(int left, int right);
typedef void (*IntVisitor)(int *value, void *context);

int add(int left, int right);
int subtract(int left, int right);
int multiply(int left, int right);
int apply_operation(BinaryOperation operation, int left, int right, int *result);
void visit_ints(int *values, size_t count, IntVisitor visitor, void *context);
int compare_int_ascending(const void *left, const void *right);

/* 这是“返回 int* 的函数”，也就是常说的指针函数。 */
int *create_int(int value);

#endif

