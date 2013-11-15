#include "queue.h"

void lockfree_queue_init(lockfree_queue_t *queue)
{
    /* Setup the sentinel nodes. */
    queue->q_head = &queue->_sentinel_head;
    queue->_sentinel_head.n_next = &queue->_sentinel_tail;
    queue->_sentinel_tail.n_next = 0;
}

void lockfree_queue_enqueue(lockfree_queue_t *q, void *v)
{
    lockfree_qnode_t *new_node = qnode_allocator();
    new_node->n_value = v;
    new_node->n_next = 0;

    /* TODO: Enqueue! */
}
