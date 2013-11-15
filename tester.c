#include "queue.h"
#include "malloc_node_allocator.h"

lockfree_qnode_t *(*qnode_allocator)(void) = &malloc_node_allocator;
void(*qnode_deallocator)(lockfree_qnode_t *) = &malloc_node_deallocator;

int main() {

    /* TODO: Test */

}
