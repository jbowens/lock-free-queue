#pragma once

#include "atomic.h"

/**
 * A lock-free queue implementation.
 *
 * @author jbowens
 */
typedef struct lockfree_qnode_t {
    void *n_value;
    struct lockfree_qnode_t *n_next;
} lockfree_qnode_t;

typedef struct {
    lockfree_qnode_t *q_head;
    lockfree_qnode_t _sentinel_head;
    lockfree_qnode_t _sentinel_tail;
} lockfree_queue_t;

/**
 * Memory allocator functions. This queue is intended to be used in the
 * Weenix kernel, so we're going to use a slab allocator for allocating
 * nodes. However, when writing the queue, I'm going to test in userland
 * where I'll use malloc. To make the transition easier, this is a fp to
 * some function that will handle allocating new nodes. In this case, it's
 * just a fp to wrapper around malloc.
 */
extern lockfree_qnode_t *(*qnode_allocator)(void);
extern void(*qnode_deallocator)(lockfree_qnode_t *);

/**
 * Initializes a new lockfree queue.
 */
void lockfree_queue_init(lockfree_queue_t *queue);
