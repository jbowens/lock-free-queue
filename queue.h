#pragma once

#include "atomic.h"

/**
 * A lock-free queue implementation.
 *
 * @author jbowens
 */
typedef struct {
    void *n_value;
    struct lockfree_qnode_t *n_next;
} lockfree_qnode_t;

typedef struct {
    lockfree_qnode_t *head;
    lockfree_qnode_t _sentinel_head;
    lockfree_qnode_t _sentinel_tail;
} lockfree_queue_t;

