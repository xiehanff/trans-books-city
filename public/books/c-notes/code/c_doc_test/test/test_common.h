#ifndef C_DOC_TEST_COMMON_H
#define C_DOC_TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>

#define TEST_ASSERT(condition)                                             \
    do {                                                                    \
        if (!(condition)) {                                                 \
            fprintf(stderr, "断言失败: %s (%s:%d)\n",                       \
                    #condition, __FILE__, __LINE__);                       \
            exit(EXIT_FAILURE);                                             \
        }                                                                   \
    } while (0)

#endif

