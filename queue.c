#include "queue.h"

void lockfree_queue_init(lockfree_queue_t *queue)
{
    /* Setup the sentinel nodes. */
    queue->q_head = &queue->_sentinel_head;
    queue->_sentinel_head.n_next = &queue->_sentinel_tail;
    queue->_sentinel_tail.n_next = 0;
}
