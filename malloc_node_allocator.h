#include <stdlib.h>
#include <assert.h>
#include "lockfree_queue.h"

/**
 * A node allocator that uses malloc.
 */
lockfree_qnode_t *malloc_node_allocator()
{
    return (lockfree_qnode_t *) malloc(sizeof(lockfree_qnode_t));
}

/**
 * A node allocator that uses free.
 */
void malloc_node_deallocator(lockfree_qnode_t *qnode)
{
    free(qnode);
}

