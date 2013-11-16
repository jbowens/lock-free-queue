#include "queue.h"

void lockfree_queue_init(lockfree_queue_t *queue)
{
    /* Setup the sentinel nodes. */
    queue->q_head = &queue->_starting_sentinel_head;
    queue->q_tail = &queue->_starting_sentinel_head;
    queue->_starting_sentinel_head.n_next = 0;
}

void lockfree_queue_enqueue(lockfree_queue_t *q, void *v)
{
    lockfree_qnode_t *new_node = qnode_allocator();
    new_node->n_value = v;
    new_node->n_next = 0;

    /* Keep attempting to enqueue. */
    while (1) {
        lockfree_qnode_t *last = q->q_tail;
        lockfree_qnode_t *next = (lockfree_qnode_t *) last->n_next;

        if (last == q->q_tail) {
            if (next == 0) {
                /* Try and atomically thread our node onto the queue. */
                if (compare_and_set(&last->n_next, next, new_node)) {
                    /* Yay, we successfully threaded ourselves onto the queue. Now,
                     * we must update the tail. */
                    compare_and_set(&q->q_tail, last, new_node);
                    return;
                }
            } else {
                /* Someone beat us to the punch. Help them out by
                 * updating the tail. */
                compare_and_set(&q->q_tail, last, next);
            }
        }
    }
}

void *lockfree_queue_dequeue(lockfree_queue_t *q)
{
    /* Keep attempting to dequeue. */
    while (1) {
        lockfree_qnode_t *first = q->q_head;
        lockfree_qnode_t *last = q->q_tail;
        lockfree_qnode_t *next = (lockfree_qnode_t *) first->n_next;
        if (first == q->q_head) {
            if (first == last) {
                if (next == 0) {
                    /* The queue is empty. Return 0. */
                    return 0;
                }
                else {
                    /* The tail hasn't been updated yet by a slow 
                     * enqueuer. Let's help them out and fix it. */
                    compare_and_set(&q->q_tail, last, next);
                }
            } else {
                void *value = next->n_value;
                /* Try and remove it from the queue. */
                if (compare_and_set(&q->q_head, first, next)) {
                    return value;
                }
            }
        }
    }
}
