#ifndef C_DOC_TEST_LINKED_LIST_H
#define C_DOC_TEST_LINKED_LIST_H

#include <stddef.h>

typedef struct ListNode {
    int value;
    struct ListNode *next;
} ListNode;

ListNode *list_node_create(int value);
int list_push_front(ListNode **head, int value);
int list_push_back(ListNode **head, int value);
const ListNode *list_find(const ListNode *head, int value);
int list_remove_first(ListNode **head, int value);
void list_reverse(ListNode **head);
size_t list_length(const ListNode *head);
void list_free(ListNode **head);

#endif

