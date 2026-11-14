#ifndef C_DOC_TEST_STACK_QUEUE_H
#define C_DOC_TEST_STACK_QUEUE_H

#include <stddef.h>

#define STACK_MAX_SIZE 100
#define QUEUE_MAX_SIZE 100
#define CIRCULAR_QUEUE_STORAGE 5

typedef struct {
    int data[STACK_MAX_SIZE];
    size_t size;
} Stack;

void stack_init(Stack *stack);
int stack_is_empty(const Stack *stack);
int stack_is_full(const Stack *stack);
int stack_push(Stack *stack, int value);
int stack_pop(Stack *stack, int *value);
int stack_peek(const Stack *stack, int *value);
int is_bracket_match(const char *expression);

typedef struct {
    int data[QUEUE_MAX_SIZE];
    size_t front;
    size_t rear;
} Queue;

void queue_init(Queue *queue);
int queue_is_empty(const Queue *queue);
int queue_is_full(const Queue *queue);
int queue_enqueue(Queue *queue, int value);
int queue_dequeue(Queue *queue, int *value);
int queue_front(const Queue *queue, int *value);

typedef struct {
    int data[CIRCULAR_QUEUE_STORAGE];
    size_t front;
    size_t rear;
} CircularQueue;

void cqueue_init(CircularQueue *queue);
int cqueue_is_empty(const CircularQueue *queue);
int cqueue_is_full(const CircularQueue *queue);
int cqueue_enqueue(CircularQueue *queue, int value);
int cqueue_dequeue(CircularQueue *queue, int *value);
int cqueue_front(const CircularQueue *queue, int *value);

typedef struct {
    Stack in;
    Stack out;
    size_t size;
} QueueByStacks;

void qbs_init(QueueByStacks *queue);
int qbs_enqueue(QueueByStacks *queue, int value);
int qbs_dequeue(QueueByStacks *queue, int *value);
int qbs_front(const QueueByStacks *queue, int *value);
int qbs_is_empty(const QueueByStacks *queue);

#endif
