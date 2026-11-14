#include "control_flow.h"

#include <limits.h>

int control_grade_level(int score)
{
    if (score >= 90) {
        return 3;
    }
    if (score >= 80) {
        return 2;
    }
    if (score >= 60) {
        return 1;
    }
    return 0;
}

int control_day_kind(int day)
{
    switch (day) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        return 1;
    case 6:
    case 7:
        return 2;
    default:
        return 0;
    }
}

long long control_sum_while(int limit)
{
    long long total = 0;
    int value = 0;

    while (value < limit) {
        total += value;
        ++value;
    }
    return total;
}

long long control_sum_do_while(int start, int stop)
{
    long long total = 0;
    int value = start;

    if (start >= stop) {
        return 0;
    }
    do {
        total += value;
        ++value;
    } while (value < stop);
    return total;
}

long long control_sum_for(int limit)
{
    long long total = 0;

    for (int value = 0; value < limit; ++value) {
        total += value;
    }
    return total;
}

size_t control_count_odd(const int *values, size_t count)
{
    size_t odd_count = 0U;

    if (values == NULL && count != 0U) {
        return 0U;
    }
    for (size_t i = 0U; i < count; ++i) {
        if (values[i] % 2 == 0) {
            continue;
        }
        ++odd_count;
    }
    return odd_count;
}

int control_find_first(const int *values, size_t count, int target)
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
