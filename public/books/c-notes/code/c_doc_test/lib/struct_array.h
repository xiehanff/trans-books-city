#ifndef C_DOC_TEST_STRUCT_ARRAY_H
#define C_DOC_TEST_STRUCT_ARRAY_H

#include <stddef.h>

typedef struct {
    char name[20];
    int age;
    float score;
} Student;

typedef struct {
    int year;
    int month;
    int day;
} Date;

typedef struct {
    char name[20];
    Date birthday;
    float salary;
} Employee;

int student_init(Student *student, const char *name, int age, float score);
int student_add_age(Student *student);
const Student *student_find(const Student *students, size_t count,
                            const char *name);
int student_average_score(const Student *students, size_t count,
                          float *average);

#endif
