#ifndef C_DOC_CONTROL_FLOW_H
#define C_DOC_CONTROL_FLOW_H

#include <stddef.h>

int control_grade_level(int score);
int control_day_kind(int day);
long long control_sum_while(int limit);
long long control_sum_do_while(int start, int stop);
long long control_sum_for(int limit);
size_t control_count_odd(const int *values, size_t count);
int control_find_first(const int *values, size_t count, int target);

#endif
