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
    stamped_ref_init(&sentinel->n_next, 0);
    stamped_ref_init(&q->q_head, sentinel);
    stamped_ref_init(&q->q_tail, sentinel);
}

void lockfree_queue_enqueue(lockfree_queue_t *q, void *v)
{
    lockfree_qnode_t *new_node = alloc_qnode();
    new_node->n_value = v;
    stamped_ref_init(&new_node->n_next, 0);

    /* Keep attempting to enqueue. */
    while (1) {
        uint32_t last_stamp, next_stamp;
        lockfree_qnode_t *last = (lockfree_qnode_t *) stamped_ref_get(&q->q_tail, &last_stamp);
        lockfree_qnode_t *next = (lockfree_qnode_t *) stamped_ref_get(&last->n_next, &next_stamp);

        uint32_t new_stamp;
        if (last == q->q_tail.ptr) {
            if (next == 0) {
                new_stamp = next_stamp + 1;
                /* Try and atomically thread our node onto the queue. */
                if (stamped_ref_cas(&last->n_next, next, next_stamp, new_node, new_stamp)) {
                    new_stamp = last_stamp + 1;
                    /* Yay, we successfully threaded ourselves onto the queue. Now,
                     * we must update the tail. */
                    stamped_ref_cas(&q->q_tail, last, last_stamp, new_node, new_stamp);
                    return;
                }
            } else {
                new_stamp = last_stamp + 1;
                /* Someone beat us to the punch. Help them out by
                 * updating the tail. */
                stamped_ref_cas(&q->q_tail, last, last_stamp, next, new_stamp);
            }
        }
    }
}

void *lockfree_queue_dequeue(lockfree_queue_t *q)
{
    /* Keep attempting to dequeue. */
    while (1) {
        uint32_t first_stamp, last_stamp, next_stamp;
        lockfree_qnode_t *first = (lockfree_qnode_t *) stamped_ref_get(&q->q_head, &first_stamp);
        lockfree_qnode_t *last = (lockfree_qnode_t *) stamped_ref_get(&q->q_tail, &last_stamp);
        lockfree_qnode_t *next = (lockfree_qnode_t *) stamped_ref_get(&first->n_next, &next_stamp);

        uint32_t new_stamp;
        if (first == q->q_head.ptr) {
            if (first == last) {
                if (next == 0) {
                    /* The queue is empty. Return 0. */
                    return 0;
                }
                else {
                    new_stamp = last_stamp + 1;
                    /* The tail hasn't been updated yet by a slow 
                     * enqueuer. Let's help them out and fix it. */
                    stamped_ref_cas(&q->q_tail, last, last_stamp, next, new_stamp);
                }
            } else {
                void *value = next->n_value;
                new_stamp = first_stamp + 1;
                /* Try and remove it from the queue. */
                if (stamped_ref_cas(&q->q_head, first, first_stamp, next, new_stamp)) {
                    return value;
                }
            }
        }
    }
}
