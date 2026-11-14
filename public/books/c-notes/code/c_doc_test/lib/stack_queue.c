#include "stack_queue.h"

static int is_opening_bracket(char character) {
    return character == '(' || character == '[' || character == '{';
}

static int is_closing_bracket(char character) {
    return character == ')' || character == ']' || character == '}';
}

static int brackets_match(char opening, char closing) {
    return (opening == '(' && closing == ')') ||
           (opening == '[' && closing == ']') ||
           (opening == '{' && closing == '}');
}

void stack_init(Stack *stack) {
    if (stack != NULL) {
        stack->size = 0;
    }
}

int stack_is_empty(const Stack *stack) {
    return stack == NULL || stack->size == 0;
}

int stack_is_full(const Stack *stack) {
    return stack != NULL && stack->size == STACK_MAX_SIZE;
}

int stack_push(Stack *stack, int value) {
    if (stack == NULL || stack_is_full(stack)) {
        return 0;
    }
    stack->data[stack->size++] = value;
    return 1;
}

int stack_pop(Stack *stack, int *value) {
    if (stack == NULL || stack->size == 0) {
        return 0;
    }
    stack->size--;
    if (value != NULL) {
        *value = stack->data[stack->size];
    }
    return 1;
}

int stack_peek(const Stack *stack, int *value) {
    if (stack == NULL || value == NULL || stack->size == 0) {
        return 0;
    }
    *value = stack->data[stack->size - 1];
    return 1;
}

int is_bracket_match(const char *expression) {
    if (expression == NULL) {
        return 0;
    }

    Stack stack;
    stack_init(&stack);
    for (size_t i = 0; expression[i] != '\0'; i++) {
        char character = expression[i];
        if (is_opening_bracket(character)) {
            if (!stack_push(&stack, (unsigned char)character)) {
                return 0;
            }
        } else if (is_closing_bracket(character)) {
            int opening = 0;
            if (!stack_pop(&stack, &opening) ||
                !brackets_match((char)opening, character)) {
                return 0;
            }
        }
    }
    return stack_is_empty(&stack);
}

void queue_init(Queue *queue) {
    if (queue != NULL) {
        queue->front = 0;
        queue->rear = 0;
    }
}

int queue_is_empty(const Queue *queue) {
    return queue == NULL || queue->front == queue->rear;
}

int queue_is_full(const Queue *queue) {
    return queue != NULL && queue->rear == QUEUE_MAX_SIZE;
}

int queue_enqueue(Queue *queue, int value) {
    if (queue == NULL || queue_is_full(queue)) {
        return 0;
    }
    queue->data[queue->rear++] = value;
    return 1;
}

int queue_dequeue(Queue *queue, int *value) {
    if (queue == NULL || queue_is_empty(queue)) {
        return 0;
    }
    if (value != NULL) {
        *value = queue->data[queue->front];
    }
    queue->front++;
    return 1;
}

int queue_front(const Queue *queue, int *value) {
    if (queue == NULL || value == NULL || queue_is_empty(queue)) {
        return 0;
    }
    *value = queue->data[queue->front];
    return 1;
}

void cqueue_init(CircularQueue *queue) {
    if (queue != NULL) {
        queue->front = 0;
        queue->rear = 0;
    }
}

int cqueue_is_empty(const CircularQueue *queue) {
    return queue == NULL || queue->front == queue->rear;
}

int cqueue_is_full(const CircularQueue *queue) {
    return queue != NULL &&
           (queue->rear + 1) % CIRCULAR_QUEUE_STORAGE == queue->front;
}

int cqueue_enqueue(CircularQueue *queue, int value) {
    if (queue == NULL || cqueue_is_full(queue)) {
        return 0;
    }
    queue->data[queue->rear] = value;
    queue->rear = (queue->rear + 1) % CIRCULAR_QUEUE_STORAGE;
    return 1;
}

int cqueue_dequeue(CircularQueue *queue, int *value) {
    if (queue == NULL || cqueue_is_empty(queue)) {
        return 0;
    }
    if (value != NULL) {
        *value = queue->data[queue->front];
    }
    queue->front = (queue->front + 1) % CIRCULAR_QUEUE_STORAGE;
    return 1;
}

int cqueue_front(const CircularQueue *queue, int *value) {
    if (queue == NULL || value == NULL || cqueue_is_empty(queue)) {
        return 0;
    }
    *value = queue->data[queue->front];
    return 1;
}

void qbs_init(QueueByStacks *queue) {
    if (queue == NULL) {
        return;
    }
    stack_init(&queue->in);
    stack_init(&queue->out);
    queue->size = 0;
}

int qbs_enqueue(QueueByStacks *queue, int value) {
    if (queue == NULL || queue->size == STACK_MAX_SIZE ||
        !stack_push(&queue->in, value)) {
        return 0;
    }
    queue->size++;
    return 1;
}

static void qbs_move_to_out(QueueByStacks *queue) {
    if (stack_is_empty(&queue->out)) {
        int value = 0;
        while (stack_pop(&queue->in, &value)) {
            (void)stack_push(&queue->out, value);
        }
    }
}

int qbs_dequeue(QueueByStacks *queue, int *value) {
    if (queue == NULL || queue->size == 0) {
        return 0;
    }
    qbs_move_to_out(queue);
    if (!stack_pop(&queue->out, value)) {
        return 0;
    }
    queue->size--;
    return 1;
}

int qbs_front(const QueueByStacks *queue, int *value) {
    if (queue == NULL || value == NULL || queue->size == 0) {
        return 0;
    }

    QueueByStacks copy = *queue;
    qbs_move_to_out(&copy);
    return stack_peek(&copy.out, value);
}

int qbs_is_empty(const QueueByStacks *queue) {
    return queue == NULL || queue->size == 0;
}
