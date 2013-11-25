#include <stdlib.h>
#include <assert.h>
#include "lockfree_queue.h"

#if __SIZEOF_POINTER__ == 4
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

#elif __SIZEOF_POINTER__ == 8
/**
 * A node allocator that uses malloc, and makes sure the alignment
 * of returned nodes is such that cmpxchg16b does not fail.
 */
lockfree_qnode_t *malloc_node_allocator()
{
    lockfree_qnode_t *unaligned = malloc(16 + sizeof(lockfree_qnode_t));
    lockfree_qnode_t *aligned;
    if ((uintptr_t) unaligned & 0x8) {
        *((void **)unaligned + 1) = unaligned;
        aligned = (lockfree_qnode_t *)((void **)unaligned + 2);
    } else {
        assert(!((uintptr_t)unaligned & 0xF));
        *(void **)unaligned = unaligned;
        aligned = (lockfree_qnode_t *)((void **)unaligned + 1);
    }
    assert(!((uintptr_t)(&aligned->n_next) & 0xF));
    return aligned;
}

/**
 * A node allocator that uses free, and frees the original pointer
 * that malloc returned.
 */
void malloc_node_deallocator(lockfree_qnode_t *qnode)
{
    free(*((void **)qnode - 1));
}
#else
#error Nonsensical pointer size!
#endif

