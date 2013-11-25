#include "lockfree_queue.h"
#include "lockfree_reapd.h"


void lockfree_queues_initall()
{
    /* Create a thread local key to be used by the hazard ptr. */
    pthread_key_create(&hazard_ptr_entry_key, NULL);

    free_qnodes_head.lffn_next = 0;
    q_hazard_chain.ht_next_table = 0;

    qnode_reapd_attr.lfra_hazard_table = &q_hazard_chain;
    qnode_reapd_attr.lfra_free_func = (void(*)(void *)) qnode_deallocator;
    qnode_reapd_attr.lfra_free_list = &free_qnodes_head;
    lf_reaper_t *reaper = lockfree_reapd_spawn(&qnode_reapd_attr);
}

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

/**
 * Covers in the hazard table and dereferences the given stamped reference.
 */
static lockfree_qnode_t *lockfree_queue_get_and_cover(lockfree_queue_t *q,
        volatile stamped_ref_t *ref, uint32_t *final_stamp)
{
    hazard_entry_t *entry = pthread_getspecific(hazard_ptr_entry_key);
    if (!entry) {
        /* We haven't gotten our hazard table entry yet. Let's make it. */
        uint32_t my_tid = __sync_fetch_and_add(&q->q_next_tid, 1);
        entry = hazard_ptr_getentry(&q_hazard_chain, my_tid);
        pthread_setspecific(hazard_ptr_entry_key, entry);
    }

    /* Keep trying until we're successful. */
    uint32_t stamp;
    uint32_t verify_stamp;
    for (;;) {
        /* Deference the stamped reference. */
        lockfree_qnode_t *node = (lockfree_qnode_t *) stamped_ref_get(ref, &stamp);
        /* Cover the node in the hazard table. */
        hazard_ptr_add(entry, node);

        /* Verify that the reference didn't change while we were covering it. */
        lockfree_qnode_t *verify_node = (lockfree_qnode_t *) stamped_ref_get(ref, &verify_stamp);
        
        if (node == verify_node && stamp == verify_stamp) {
            /* We succeeded! We can return this node and we're guaranteed that no one will free it
             * out from underneath us. */
            *final_stamp = stamp;
            return node;
        }
       
        /* We failed. Remove the ptr from our table and try again. */
        hazard_ptr_remove(entry, node);
    }
}

static void lockfree_queue_uncover(void *ptr)
{
    hazard_entry_t *entry = pthread_getspecific(hazard_ptr_entry_key);
    /* TODO: Assert that entry != NULL */

    hazard_ptr_remove(entry, ptr);
}

void lockfree_queue_enqueue(lockfree_queue_t *q, void *v)
{
    lockfree_qnode_t *new_node = alloc_qnode();
    new_node->n_value = v;
    stamped_ref_init(&new_node->n_next, 0);

    /* Keep attempting to enqueue. */
    while (1) {
        uint32_t last_stamp, next_stamp;
        lockfree_qnode_t *last = lockfree_queue_get_and_cover(q, &q->q_tail, &last_stamp);
        lockfree_qnode_t *next = lockfree_queue_get_and_cover(q, &last->n_next, &next_stamp);

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
                    lockfree_queue_uncover(last);
                    lockfree_queue_uncover(next);
                    return;
                }
            } else {
                new_stamp = last_stamp + 1;
                /* Someone beat us to the punch. Help them out by
                 * updating the tail. */
                stamped_ref_cas(&q->q_tail, last, last_stamp, next, new_stamp);
            }
        }
        lockfree_queue_uncover(last);
        lockfree_queue_uncover(next);
    }
}

void *lockfree_queue_dequeue(lockfree_queue_t *q)
{
    /* Keep attempting to dequeue. */
    while (1) {
        uint32_t first_stamp, last_stamp, next_stamp;
        lockfree_qnode_t *first = lockfree_queue_get_and_cover(q, &q->q_head, &first_stamp);
        lockfree_qnode_t *last = lockfree_queue_get_and_cover(q, &q->q_tail, &last_stamp);
        lockfree_qnode_t *next = lockfree_queue_get_and_cover(q, &first->n_next, &next_stamp);

        uint32_t new_stamp;
        if (first == q->q_head.ptr) {
            if (first == last) {
                if (next == 0) {
                    /* The queue is empty. Return 0. */
                    lockfree_queue_uncover(first);
                    lockfree_queue_uncover(last);
                    lockfree_queue_uncover(next);
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
                    lockfree_queue_uncover(first);
                    lockfree_queue_uncover(last);
                    lockfree_queue_uncover(next);

                    if (hazard_table_search(&q_hazard_chain, first)) {
                        /* Some other thread still has a reference to this node. We should
                         * put it on the free list so that the reaper will reap this dead
                         * node. */
                        lockfree_freenode_t *free_node;
                        lockfree_freenode_t *next_node;
                        do {
                            free_node = (lockfree_freenode_t *) first;
                            next_node = free_qnodes_head.lffn_next;
                            free_node->lffn_next = next_node;
                        } while (!compare_and_set(&free_qnodes_head.lffn_next, next_node, free_node));
                    } else {
                        /* No one else is looking at this node right now, so we can safely
                         * free it ourselves. */
                        qnode_deallocator(first);
                    }

                    return value;
                }
            }
        }

        lockfree_queue_uncover(first);
        lockfree_queue_uncover(last);
        lockfree_queue_uncover(next);
    }
}
