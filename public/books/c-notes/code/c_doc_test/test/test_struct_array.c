#include "test_common.h"

#include <stddef.h>
#include <string.h>

#include "struct_array.h"

void test_struct_array(void) {
    /* 第09章：结构体初始化、点/箭头访问，以及结构体赋值。 */
    Student first = {"张三", 20, 85.5F};
    Student copied = first;
    TEST_ASSERT(strcmp(copied.name, "张三") == 0);
    TEST_ASSERT(copied.age == 20 && copied.score == 85.5F);

    Student student;
    TEST_ASSERT(student_init(&student, "李四", 19, 90.0F));
    Student *pointer = &student;
    pointer->age = 20;
    TEST_ASSERT(student.age == 20);
    TEST_ASSERT(student_add_age(&student) && student.age == 21);
    TEST_ASSERT(!student_init(&student, "this name is too long", 1, 1.0F));
    TEST_ASSERT(!student_add_age(NULL));

    /* 第09章和第10章：结构体数组、嵌套结构体以及显式传入数组长度。 */
    Student students[3];
    TEST_ASSERT(student_init(&students[0], "张三", 20, 85.5F));
    TEST_ASSERT(student_init(&students[1], "李四", 19, 90.0F));
    TEST_ASSERT(student_init(&students[2], "王五", 21, 78.5F));
    const Student *found = student_find(students, sizeof students / sizeof students[0],
                                        "李四");
    TEST_ASSERT(found == &students[1]);
    TEST_ASSERT(student_find(students, 3, "不存在") == NULL);

    float average = 0.0F;
    TEST_ASSERT(student_average_score(students, sizeof students / sizeof students[0],
                                      &average));
    TEST_ASSERT(average > 84.6F && average < 84.7F);
    TEST_ASSERT(!student_average_score(NULL, 0, &average));

    Employee employee = {
        .name = "赵六",
        .birthday = {.year = 1990, .month = 5, .day = 15},
        .salary = 8000.0F
    };
    Employee *employee_pointer = &employee;
    TEST_ASSERT(employee_pointer->birthday.year == 1990);
    TEST_ASSERT(employee.birthday.month == 5 && employee.birthday.day == 15);

    /* 第09章：offsetof 用于观察成员布局，不假设结构体没有对齐填充。 */
    TEST_ASSERT(offsetof(Student, name) == 0);
    TEST_ASSERT(offsetof(Student, age) > offsetof(Student, name));
    TEST_ASSERT(offsetof(Student, score) > offsetof(Student, age));
}
