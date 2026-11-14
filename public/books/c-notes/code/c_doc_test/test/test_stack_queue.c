#include "test_common.h"

#include "stack_queue.h"

void test_stack_queue(void) {
    /* 第13章：数组栈的后进先出，以及空栈/满栈边界。 */
    Stack stack;
    stack_init(&stack);
    TEST_ASSERT(stack_is_empty(&stack));
    TEST_ASSERT(!stack_is_full(&stack));
    TEST_ASSERT(!stack_pop(&stack, NULL));
    TEST_ASSERT(!stack_peek(&stack, NULL));

    for (int i = 0; i < STACK_MAX_SIZE; i++) {
        TEST_ASSERT(stack_push(&stack, i));
    }
    TEST_ASSERT(stack_is_full(&stack));
    TEST_ASSERT(!stack_push(&stack, STACK_MAX_SIZE));

    int value = 0;
    TEST_ASSERT(stack_peek(&stack, &value) && value == STACK_MAX_SIZE - 1);
    for (int i = STACK_MAX_SIZE - 1; i >= 0; i--) {
        TEST_ASSERT(stack_pop(&stack, &value) && value == i);
    }
    TEST_ASSERT(stack_is_empty(&stack));

    /* 第13章：栈用于括号匹配，忽略非括号字符。 */
    TEST_ASSERT(is_bracket_match("a * (b + [c - {d}])"));
    TEST_ASSERT(!is_bracket_match("([)]"));
    TEST_ASSERT(!is_bracket_match("}"));
    TEST_ASSERT(is_bracket_match("no brackets"));
    TEST_ASSERT(!is_bracket_match(NULL));

    /* 第13章：普通数组队列的先进先出和容量边界。 */
    Queue queue;
    queue_init(&queue);
    TEST_ASSERT(queue_is_empty(&queue));
    TEST_ASSERT(!queue_dequeue(&queue, &value));
    for (int i = 0; i < QUEUE_MAX_SIZE; i++) {
        TEST_ASSERT(queue_enqueue(&queue, i));
    }
    TEST_ASSERT(queue_is_full(&queue));
    TEST_ASSERT(!queue_enqueue(&queue, 100));
    TEST_ASSERT(queue_front(&queue, &value) && value == 0);
    for (int i = 0; i < QUEUE_MAX_SIZE; i++) {
        TEST_ASSERT(queue_dequeue(&queue, &value) && value == i);
    }
    TEST_ASSERT(queue_is_empty(&queue));

    /* 第13章：循环队列出队后复用数组前部空间（容量为存储单元数减一）。 */
    CircularQueue circular;
    cqueue_init(&circular);
    TEST_ASSERT(cqueue_is_empty(&circular));
    for (int i = 1; i <= CIRCULAR_QUEUE_STORAGE - 1; i++) {
        TEST_ASSERT(cqueue_enqueue(&circular, i));
    }
    TEST_ASSERT(cqueue_is_full(&circular));
    TEST_ASSERT(!cqueue_enqueue(&circular, 99));
    TEST_ASSERT(cqueue_front(&circular, &value) && value == 1);
    TEST_ASSERT(cqueue_dequeue(&circular, &value) && value == 1);
    TEST_ASSERT(cqueue_enqueue(&circular, 99));
    TEST_ASSERT(cqueue_dequeue(&circular, &value) && value == 2);
    TEST_ASSERT(cqueue_dequeue(&circular, &value) && value == 3);
    TEST_ASSERT(cqueue_dequeue(&circular, &value) && value == 4);
    TEST_ASSERT(cqueue_dequeue(&circular, &value) && value == 99);
    TEST_ASSERT(cqueue_is_empty(&circular));
    TEST_ASSERT(!cqueue_dequeue(&circular, &value));

    /* 第13章：两个栈倒置后实现队列的先进先出。 */
    QueueByStacks by_stacks;
    qbs_init(&by_stacks);
    TEST_ASSERT(qbs_is_empty(&by_stacks));
    TEST_ASSERT(qbs_enqueue(&by_stacks, 10));
    TEST_ASSERT(qbs_enqueue(&by_stacks, 20));
    TEST_ASSERT(qbs_enqueue(&by_stacks, 30));
    TEST_ASSERT(qbs_front(&by_stacks, &value) && value == 10);
    TEST_ASSERT(qbs_dequeue(&by_stacks, &value) && value == 10);
    TEST_ASSERT(qbs_dequeue(&by_stacks, &value) && value == 20);
    TEST_ASSERT(qbs_enqueue(&by_stacks, 40));
    TEST_ASSERT(qbs_dequeue(&by_stacks, &value) && value == 30);
    TEST_ASSERT(qbs_dequeue(&by_stacks, &value) && value == 40);
    TEST_ASSERT(qbs_is_empty(&by_stacks));
    TEST_ASSERT(!qbs_dequeue(&by_stacks, &value));
}
