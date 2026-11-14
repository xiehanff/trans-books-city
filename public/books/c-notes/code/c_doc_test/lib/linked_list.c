#include "linked_list.h"

#include <stdlib.h>

ListNode *list_node_create(int value) {
    ListNode *node = malloc(sizeof *node);
    if (node == NULL) {
        return NULL;
    }

    node->value = value;
    node->next = NULL;
    return node;
}

int list_push_front(ListNode **head, int value) {
    if (head == NULL) {
        return 0;
    }

    ListNode *node = list_node_create(value);
    if (node == NULL) {
        return 0;
    }

    node->next = *head;
    *head = node;
    return 1;
}

int list_push_back(ListNode **head, int value) {
    if (head == NULL) {
        return 0;
    }

    ListNode *node = list_node_create(value);
    if (node == NULL) {
        return 0;
    }

    if (*head == NULL) {
        *head = node;
        return 1;
    }

    ListNode *current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = node;
    return 1;
}

const ListNode *list_find(const ListNode *head, int value) {
    for (const ListNode *current = head;
         current != NULL;
         current = current->next) {
        if (current->value == value) {
            return current;
        }
    }
    return NULL;
}

int list_remove_first(ListNode **head, int value) {
    if (head == NULL) {
        return 0;
    }

    /* link 指向“保存当前节点地址的指针”，因此头节点也能统一处理。 */
    ListNode **link = head;
    while (*link != NULL) {
        if ((*link)->value == value) {
            ListNode *removed = *link;
            *link = removed->next;
            free(removed);
            return 1;
        }
        link = &(*link)->next;
    }
    return 0;
}

void list_reverse(ListNode **head) {
    if (head == NULL) {
        return;
    }

    ListNode *previous = NULL;
    ListNode *current = *head;
    while (current != NULL) {
        ListNode *next = current->next;
        current->next = previous;
        previous = current;
        current = next;
    }
    *head = previous;
}

size_t list_length(const ListNode *head) {
    size_t length = 0;
    for (const ListNode *current = head;
         current != NULL;
         current = current->next) {
        length++;
    }
    return length;
}

void list_free(ListNode **head) {
    if (head == NULL) {
        return;
    }

    ListNode *current = *head;
    while (current != NULL) {
        ListNode *next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}

