#include "struct_array.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

int student_init(Student *student, const char *name, int age, float score) {
    if (student == NULL || name == NULL) {
        return 0;
    }

    size_t name_length = strlen(name);
    if (name_length >= sizeof student->name) {
        return 0;
    }

    memcpy(student->name, name, name_length + 1);
    student->age = age;
    student->score = score;
    return 1;
}

int student_add_age(Student *student) {
    if (student == NULL || student->age == INT_MAX) {
        return 0;
    }
    student->age++;
    return 1;
}

const Student *student_find(const Student *students, size_t count,
                            const char *name) {
    if (students == NULL || name == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < count; i++) {
        if (strcmp(students[i].name, name) == 0) {
            return &students[i];
        }
    }
    return NULL;
}

int student_average_score(const Student *students, size_t count,
                          float *average) {
    if (students == NULL || count == 0 || average == NULL) {
        return 0;
    }

    float total = 0.0F;
    for (size_t i = 0; i < count; i++) {
        total += students[i].score;
    }
    *average = total / (float)count;
    return 1;
}
