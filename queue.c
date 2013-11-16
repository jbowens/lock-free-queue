#include "queue.h"

void lockfree_queue_init(lockfree_queue_t *queue)
{
    /* Setup the sentinel nodes. */
    queue->q_head = &queue->_sentinel_head;
    queue->q_tail = &queue->_sentinel_head;
    queue->_sentinel_head.n_next = 0;
}

void lockfree_queue_enqueue(lockfree_queue_t *q, void *v)
{
    lockfree_qnode_t *new_node = qnode_allocator();
    new_node->n_value = v;
    new_node->n_next = 0;

    /* Keep attempting to enqueue. */
    while (true) {
        lockfree_qnode_t *last = q->q_tail;
        lockfree_qnode_t *next = last->n_next;

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
