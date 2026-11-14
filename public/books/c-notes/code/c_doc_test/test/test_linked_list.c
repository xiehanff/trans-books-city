#include "test_common.h"

#include "linked_list.h"

void test_linked_list(void) {
    ListNode *head = NULL;

    TEST_ASSERT(list_length(head) == 0);
    TEST_ASSERT(list_push_front(&head, 2));
    TEST_ASSERT(list_push_front(&head, 1));
    TEST_ASSERT(list_push_back(&head, 3));
    TEST_ASSERT(list_length(head) == 3);

    TEST_ASSERT(head != NULL && head->value == 1);
    TEST_ASSERT(head->next != NULL && head->next->value == 2);
    TEST_ASSERT(head->next->next != NULL && head->next->next->value == 3);
    TEST_ASSERT(list_find(head, 2) == head->next);
    TEST_ASSERT(list_find(head, 99) == NULL);

    TEST_ASSERT(list_remove_first(&head, 1));
    TEST_ASSERT(head != NULL && head->value == 2);
    TEST_ASSERT(list_remove_first(&head, 3));
    TEST_ASSERT(head != NULL && head->value == 2 && head->next == NULL);
    TEST_ASSERT(!list_remove_first(&head, 99));

    TEST_ASSERT(list_push_back(&head, 3));
    TEST_ASSERT(list_push_back(&head, 4));
    list_reverse(&head);
    TEST_ASSERT(head != NULL && head->value == 4);
    TEST_ASSERT(head->next != NULL && head->next->value == 3);
    TEST_ASSERT(head->next->next != NULL && head->next->next->value == 2);

    list_free(&head);
    TEST_ASSERT(head == NULL);
    list_free(&head); /* 释放后再次调用也应安全。 */

    TEST_ASSERT(!list_push_front(NULL, 1));
    TEST_ASSERT(!list_remove_first(NULL, 1));
}

