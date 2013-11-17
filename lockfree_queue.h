#pragma once

#include "hazard_ptr.h"

/**
 * A lock-free queue. This uses the algorithm outlined in Herlihy and Shavit's The
 * Art of Multiprocessor Programming.
 *
 * @author jbowens, ejcaruso
 */

/*
 * A node in the queue.
 */
typedef struct lockfree_qnode {
    void *n_value;
    volatile struct lockfree_qnode *n_next;
} lockfree_qnode_t;

/**
 * The queue data structure.
 */
typedef struct lockfree_queue {
    lockfree_qnode_t *q_head;
    lockfree_qnode_t *q_tail;
    hazard_table_t q_hazard_chain;
    uint32_t q_next_tid;
} lockfree_queue_t;

/**
 * Memory allocator functions. This queue is intended to be used in the
 * Weenix kernel, so we're going to use a slab allocator for allocating
 * nodes. However, when writing the queue, I'm going to test in userland
 * where I'll use malloc. To make the transition easier, this is a fp to
 * some function that will handle allocating new nodes. In this case, it's
 * just a fp to a wrapper around malloc.
 */
extern lockfree_qnode_t *(*qnode_allocator)(void);
extern void(*qnode_deallocator)(lockfree_qnode_t *);

/**
 * Initializes a new lockfree queue.
 */
void lockfree_queue_init(lockfree_queue_t *q);

/**
 * Enqueues an item onto the queue.
 *
 * @param q the queue on which to enqueue
 * @param v the value to enqueue
 */
void lockfree_queue_enqueue(lockfree_queue_t *q, void *v);

/**
 * Dequeues an item from the queue. If there are no items on
 * the queue, it returns 0.
 *
 * @param q the queue from which to dequeue
 */
void *lockfree_queue_dequeue(lockfree_queue_t *q);
