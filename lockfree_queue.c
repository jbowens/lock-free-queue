#include "lockfree_queue.h"
#include "lockfree_reapd.h"

/**
 * Allocates a new qnode. This function should be used whenever allocating a
 * new qnode. qnode_allocator() should NOT be called directly as it creates
 * a new qnode on every call. This function instead will recycle qnodes if
 * possible.
 */
static lockfree_qnode_t *alloc_qnode()
{
    return qnode_allocator();
}

void lockfree_queue_init(lockfree_queue_t *q)
{
    /* Setup the sentinel nodes. */
    lockfree_qnode_t *sentinel = alloc_qnode();
    sentinel->n_next = 0;
    q->q_head = sentinel;
    q->q_tail = sentinel;

    q->q_hazard_chain.ht_next_table = 0;
}

void lockfree_queue_enqueue(lockfree_queue_t *q, void *v)
{
    lockfree_qnode_t *new_node = alloc_qnode();
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
